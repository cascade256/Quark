#include "FileTree.h"
#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif
void createFileTree(const char* path, FileTreeItem* tree) {
	logI("Creating file tree: %s\n", path);
	int pathLen = strlen(path);
	tree->path = new char[pathLen + 1];
	strncpy(tree->path, path, pathLen + 1);

	const char* dirName;
#ifdef _WIN32
	dirName = &strrchr(tree->path, '\\')[1];
#else
	dirName = &strrchr(tree->path, '/')[1];
#endif
	int nameLen = strlen(dirName);
	tree->name = new char[nameLen + 1];
	strncpy(tree->name, dirName, nameLen + 1);

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

		subItems[i].name = new char[strlen(ent->d_name) + 1];
		strcpy(subItems[i].name, ent->d_name);

#ifdef _WIN32
		int itemPathLen = ent->d_namlen + pathLen + 2;//Add 2, one for the null byte, and one for the backslash
		//TODO add bounds checking
		subItems[i].path = new char[itemPathLen];
		strcpy(subItems[i].path, tree->path);
		strcat(subItems[i].path, "\\");
		strcat(subItems[i].path, ent->d_name);

#else
		int itemPathLen = strlen(ent->d_name) + pathLen + 2;//Add 2, one for the null byte, and one for the backslash
		//TODO add bounds checking
		subItems[i].path = new char[itemPathLen];
		strcpy(subItems[i].path, tree->path);
		strcat(subItems[i].path, "/");
		strcat(subItems[i].path, ent->d_name);
#endif

		subItems[i].subItemCount = 0;
		subItems[i].subItems = NULL;
		subItems[i].loaded = false;
		subItems[i].selected = false;
		logI("%s\n", subItems[i].name);
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
		if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_DOUBLE, bounds))
		{
			logI("File: %s was double clicked\n", tree->name);
			addJob(jobbedOpenFile, (void*)tree->path);
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
