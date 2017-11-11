#include "FileTree.h"
#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif

//Case insensitive string comparison
int strcasecmp(const char* a, const char* b) {
	int i = 0;
	while((a[i] != '\0') && (b[i] != '\0') && (tolower(a[i]) == tolower(b[i]))) {
		i++;
	}
	return tolower(a[i]) - tolower(b[i]);
}

void createFileTree(const char* path, FileTreeFolder* tree) {
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
}

//path must be the absolute path to a directory, not a file
void extendFileTree(FileTreeFolder* tree) {
	assert(tree->path != NULL);
	DIR* dir = opendir(tree->path);
	if (dir == NULL) {
		tree->loaded = true;
		return;
	}
	dirent* ent;

	Array<FileTreeFolder> folders;
	arrayInit(&folders);
	Array<FileTreeFile> files;
	arrayInit(&files);

	int pathLen = strlen(tree->path);

	ent = readdir(dir);
	while (ent != NULL) {
		if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
			ent = readdir(dir);
			continue;
		}

		if(ent->d_type == DT_DIR) {
			//Find the index to insert the new item
			int i = 0;
			while(i < folders.len && strcasecmp(ent->d_name, folders[i].name) > 0) {
				i++;
			}

			//Create the new FileTreeFolder
			FileTreeFolder folder;
			folder.loaded = false;
			folder.selected = false;

			folder.path = new char[strlen(ent->d_name) + pathLen + 2];//Add 2, one for the null byte, and one for the slash
			strcpy(folder.path, tree->path);
#ifdef _WIN32
			strcat(folder.path, "\\");
#else
			strcat(folder.path, "/");
#endif
			strcat(folder.path, ent->d_name);

			folder.name = new char[strlen(ent->d_name) + 1];
			strcpy(folder.name, ent->d_name);

			//Insert it into the array
			arrayInsert(&folders, i, folder);
		}
		else if(ent->d_type == DT_REG) {
			//Find the index to insert the new item
			int i = 0;
			while(i < files.len && strcasecmp(ent->d_name, files[i].name) > 0) {
				i++;
			}

			//Create the new FileTreeFile
			FileTreeFile file;
			file.selected = false;

			file.path = new char[strlen(ent->d_name) + pathLen + 2];//Add 2, one for the null byte, and one for the slash
			strcpy(file.path, tree->path);
#ifdef _WIN32
			strcat(file.path, "\\");
#else
			strcat(file.path, "/");
#endif
			strcat(file.path, ent->d_name);

			file.name = new char[strlen(ent->d_name) + 1];
			strcpy(file.name, ent->d_name);

			//Insert it into the array
			arrayInsert(&files, i, file);
		}
		else {
			logI("Unknown file system object type, skipping\n");
		}
		ent = readdir(dir);
	}

	closedir(dir);
	tree->folders = folders;
	tree->files = files;
	tree->loaded = true;
}

void destroyFileTree(FileTreeFolder* fileTree) {
	for (int i = 0; i < fileTree->folders.len; i++) {
		destroyFileTree(&fileTree->folders[i]);
	}
	if (fileTree->loaded) {
		delete[] fileTree->folders.data;
		fileTree->folders.data = NULL;
		delete[] fileTree->files.data;
		fileTree->files.data = NULL;
	}
	delete[] fileTree->name;
	fileTree->name = NULL;
	delete[] fileTree->path;
	fileTree->path = NULL;
}

void drawFileTree(nk_context* ctx, FileTreeFolder* tree, OpenFiles* openFiles) {
	if (nk_tree_push_hashed(ctx, NK_TREE_NODE, tree->name, NK_MINIMIZED, tree->path, strlen(tree->path), 0))
	{
		if (!tree->loaded) {
			extendFileTree(tree);
		}
		for (int i = 0; i < tree->folders.len; i++) {
			drawFileTree(ctx, &tree->folders[i], openFiles);
		}
		for(int i = 0; i < tree->files.len; i++) {
			struct nk_rect bounds = nk_widget_bounds(ctx);
			nk_selectable_label(ctx, tree->files[i].name, NK_TEXT_LEFT, &tree->files[i].selected);
			if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_DOUBLE, bounds))
			{
				logI("File: %s was double clicked\n", tree->files[i].name);
				addJobWithArgs(jobbedOpenFile, (void*)tree->files[i].path);
			}
		}
		nk_tree_pop(ctx);
	}
}
