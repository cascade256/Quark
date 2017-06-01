#include "PluginManager.h"

#define MAX_EXTENSION_LENGTH 12

Plugin_API api;
std::vector<Plugin>* plugins;

void registerColorizer(Colorize_Func func, const char* fileExt) {
	hashmap_put(g->colorizers, fileExt, (void*)func);
}

void registerAutocompleter(AutoComplete_Func func, const char* fileExt) {
	hashmap_put(g->autocompleters, fileExt, (void*)func);
}

Global* getGlobals() {
	return g;
}

bool loadPlugin(const char* path, Plugin* plugin) {
	logI("Loading plugin: %s\n", path);

#ifdef _WIN32
	HINSTANCE dll = LoadLibrary(path);
	if (!dll) {
		logW("Failed to load the plugin\n");
		return false;
	}

	plugin->getPluginInfo = (type_getPluginInfo)GetProcAddress(dll, "getPluginInfo");
	if (plugin->getPluginInfo == NULL) {
		return false;
	}
	plugin->initPlugin = (type_initPlugin)GetProcAddress(dll, "initPlugin");
	if (plugin->initPlugin == NULL) {
		return false;
	}
	plugin->destroyPlugin = (type_destroyPlugin)GetProcAddress(dll, "destroyPlugin");
	if (plugin->destroyPlugin == NULL) {
		return false;
	}
#else
	void* dll = dlopen(path, RTLD_NOW);
	if (!dll) {
		logW("Failed to load the plugin\n");
		return false;
	}

	plugin->getPluginInfo = (type_getPluginInfo)dlsym(dll, "getPluginInfo");
	if (plugin->getPluginInfo == NULL) {
		return false;
	}
	plugin->initPlugin = (type_initPlugin)dlsym(dll, "initPlugin");
	if (plugin->initPlugin == NULL) {
		return false;
	}
	plugin->destroyPlugin = (type_destroyPlugin)dlsym(dll, "destroyPlugin");
	if (plugin->destroyPlugin == NULL) {
		return false;
	}
#endif

	return true;
}

void setupPlugin(Plugin plugin) {
	plugin.initPlugin(api);
}

void loadPlugins() {
	plugins = new std::vector<Plugin>();
	api.registerColorizer = registerColorizer;
	api.registerAutocompleter = registerAutocompleter;
	api.getGlobalData = getGlobals;
	api.registerMenu = addMenu;
	api.registerMenuItem = addMenuItem;


	char* pluginPath = new char[1024];

#ifdef _WIN32
	GetModuleFileName(NULL, pluginPath, 1024);
	char* fileName = strrchr(pluginPath, '\\');
	strncpy(fileName, "\\plugins\\",  1024 - (fileName - pluginPath));
#elif __linux__
	int len = readlink("/proc/self/exe", pluginPath, 1024);
	pluginPath[len] = '\0';
	char* fileName = strrchr(pluginPath, '/');
	strncpy(fileName, "/plugins/", 1024 - (fileName - pluginPath));
#else
#error "Unsupported platform!"
#endif


	logI("Plugins path: %s\n", pluginPath);

	char* pluginNameLoc = &pluginPath[strlen(pluginPath)];
	int maxPluginNameLen = 1024 - (pluginNameLoc - pluginPath);

	DIR* dir = opendir(pluginPath);
	if (dir == NULL) {
		logW("The plugins folder could not be read\n");
		return;
	}
	dirent* ent;
	ent = readdir(dir);
	Plugin plugin;
	while (ent != NULL) {
		if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0) {
			ent = readdir(dir);
			continue;
		}
#ifdef _WIN32
		if (strcmp(strrchr(ent->d_name, '.'), ".dll") == 0) {
#else
		if (strcmp(strrchr(ent->d_name, '.'), ".so") == 0) {
#endif
			strncpy(pluginNameLoc, ent->d_name, maxPluginNameLen);

			if (loadPlugin(pluginPath, &plugin)) {
				plugins->push_back(plugin);
				setupPlugin(plugin);
			}
			else {
				logW("The plugin failed to load: %s\n", pluginPath);
			}
		}

		ent = readdir(dir);
	}

	delete[] pluginPath;
}

void destroyPlugins() {
	for (int i = 0; i < plugins->size(); i++) {
		(*plugins)[i].destroyPlugin(api);
	}
	delete plugins;
}
