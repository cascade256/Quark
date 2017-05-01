project('quark-core', 'cpp')

cc = meson.get_compiler('cpp')
platform = host_machine.system()

#find glfw3
glfwdep = dependency('glfw3', required: false)
if glfwdep.found() == false
	glfw_lib = cc.find_library('glfw3', dirs: meson.current_source_dir() + '/deps/libglfw3/')
	glfw_inc = include_directories('deps/libglfw3/include/')
	glfwdep = declare_dependency(dependencies: glfw_lib, include_directories: glfw_inc)
endif

if platform == 'linux'
	add_project_link_arguments('-ldl', language : 'cpp')
endif

if platform == 'windows'
	add_project_link_arguments('-lopengl32', language: 'cpp')
endif

cppplugin = shared_library('cpp_plugin', ['src/cpp_plugin/CPP_Plugin.cpp'], install : false)

executable('quark', 'src/main.cpp', 'src/hashmap.cpp','src/OpenFileManager.cpp', 'src/PluginManager.cpp', 'src/FileTree.cpp', 'src/TextEditor.cpp', 'src/AutocompleteDialog.cpp', 'src/FindDialog.cpp', 'src/gl3w.cpp', dependencies: glfwdep)

#Copy the font file to the build folder
run_command('cp', 'src/DroidSansMono.ttf', 'build/')

#Create the plugin dir
run_command('mkdir', 'build/plugins')
