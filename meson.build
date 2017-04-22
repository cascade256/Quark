project('quark-core', 'cpp')
epoxydep = dependency('epoxy')
glfwdep = dependency('glfw3')

add_project_link_arguments('-ldl', language : 'cpp')
cppplugin = shared_library('cpp_plugin', ['src/cpp_plugin/CPP_Plugin.cpp'], install : false)

executable('quark', 'src/main.cpp', 'src/hashmap.cpp','src/OpenFileManager.cpp', 'src/PluginManager.cpp', 'src/FileTree.cpp', 'src/TextEditor.cpp', 'src/AutocompleteDialog.cpp', dependencies: [epoxydep, glfwdep])


copyPlugins = custom_target('copy plugins', output: 'blank.txt', command: ['mv', '*.so', 'plugins/'])
