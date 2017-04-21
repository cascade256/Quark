#include "FileTree.h"
#include "dirent.h"

void createFileTree(const char* path, FileTreeItem* tree) {
	int pathLen = strlen(path);
	tree->path = new char[pathLen + 1];
	strcpy_s(tree->path, pathLen + 1, path);

	const char* dirName;
#ifdef _WIN32
	dirName = &strrchr(tree->path, '\\')[1];
#endif
	int nameLen = strlen(dirName);
	tree->name = new char[nameLen + 1];
	strcpy_s(tree->name, nameLen + 1, dirName);

	tree->loaded = false;
	tree->selected = false;
	tree->type = DT_DIR;
	tree->subItems = NULL;
	tree->subItemCount = 0;
}

//path must be the absolute path to a directory, not a file
void extendFileTree(FileTreeItem* tree) {
	assert(tree->path != NULL);
	DIR* dir = opendir(tree->path);
	if (dir == NULL) {
		tree->loaded = true;
		return;
	}
	dirent* ent;
	int numItems = 0;

	//Get a count of the number of items in the directory
	if (dir != NULL) {
		ent = readdir(dir);
		while (ent != NULL) {
			numItems++;
			ent = readdir(dir);
		}
	}
	rewinddir(dir);//Rewind so it is ready for the actual read loop
	numItems -= 2; //Ignore the /. and /.. folders

	int pathLen = strlen(tree->path);

	//Set up its subItems
	FileTreeItem* subItems = new FileTreeItem[numItems];

	ent = readdir(dir);
	while (ent != NULL && (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)) {
		ent = readdir(dir);
	}
	for (int i = 0; i < numItems; i++) {
		subItems[i].type = ent->d_type;

		subItems[i].name = new char[ent->d_namlen + 1];
		strcpy_s(subItems[i].name, ent->d_namlen + 1, ent->d_name);

#ifdef _WIN32
		int itemPathLen = ent->d_namlen + pathLen + 2;//Add 2, one for the null byte, and one for the backslash
		subItems[i].path = new char[itemPathLen];
		strcpy_s(subItems[i].path, itemPathLen, tree->path);
		strcat_s(subItems[i].path, itemPathLen, "\\");
		strcat_s(subItems[i].path, itemPathLen, ent->d_name);
#endif

		subItems[i].subItemCount = 0;
		subItems[i].subItems = NULL;
		subItems[i].loaded = false;
		subItems[i].selected = false;
		printf("%s\n", subItems[i].name);
		ent = readdir(dir);
		while (ent != NULL && (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)) {
			ent = readdir(dir);
		}
	}
	closedir(dir);
	tree->subItems = subItems;
	tree->subItemCount = numItems;
	tree->loaded = true;
}

void destroyFileTree(FileTreeItem* fileTree) {
	for (int i = 0; i < fileTree->subItemCount; i++) {
		destroyFileTree(&fileTree->subItems[i]);
	}
	delete[] fileTree->name;
	fileTree->name = NULL;
	delete[] fileTree->subItems;
	fileTree->subItems = NULL;
	delete[] fileTree->path;
	fileTree->path = NULL;
}

void drawFileTree(nk_context* ctx, FileTreeItem* tree, OpenFiles* files) {
	bool isDir = (tree->type & DT_DIR) == DT_DIR;
	if (!isDir) {
		struct nk_rect bounds = nk_widget_bounds(ctx);
		nk_selectable_label(ctx, tree->name, NK_TEXT_LEFT, &tree->selected);
		if ((nk_input_is_mouse_hovering_rect(&ctx->input, bounds) ||
			nk_input_is_mouse_prev_hovering_rect(&ctx->input, bounds)) &&
			nk_input_is_mouse_down(&ctx->input, NK_BUTTON_DOUBLE_CLICK))
		{
			printf("File: %s was double clicked\n", tree->name);
			openFile(files, tree->path);
		}
	}
	else {
		if (nk_tree_push_hashed(ctx, NK_TREE_NODE, tree->name, NK_MINIMIZED, tree->path, strlen(tree->path), 0))
		{
			if (!tree->loaded) {
				extendFileTree(tree);
			}
			for (int i = 0; i < tree->subItemCount; i++) {
				drawFileTree(ctx, &tree->subItems[i], files);
			}
			nk_tree_pop(ctx);
		}
	}
}