cmake_minimum_required(VERSION 3.1)

if(UNIX)
	file(COPY fonts/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/fonts/)
	#Create plugins folder
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plugins)
elseif(WIN32)
	#Copy the fonts to the binary folders
	file(COPY fonts/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/fonts/)
	file(COPY fonts/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/fonts/)
	file(COPY fonts/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel/fonts/)
	file(COPY fonts/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/fonts/)

	#Create plugins folder
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug/plugins)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release/plugins)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel/plugins)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/plugins)

	add_compile_options(/wd4244 /wd4267 /wd4800)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#Build QuarkCore
project (QuarkCore C CXX)

set(CMAKE_CXX_STANDARD 11)
include_directories(libs/libglfw3/include)

add_library(QuarkCore SHARED
		src/QuarkCore.cpp
		src/FileTree.cpp
		src/FindDialog.cpp
		src/gl3w.cpp
		src/hashmap.cpp
		src/JobManager.cpp
		src/LayoutManager.cpp
		src/Logger.cpp
		src/OpenFileManager.cpp
		src/ParsingUtils.cpp
		src/PluginManager.cpp
		src/TextEditor.cpp
		src/tinycthread.c
		src/nuklear/nuklear.cpp
		src/Theme.cpp
		src/dyad.c
)

if(UNIX)
    find_package(Threads REQUIRED)
    find_package(glfw3 REQUIRED)
    target_link_libraries(QuarkCore
			glfw
			GL
			X11
			Xrandr
			Xinerama
			Xxf86vm
			Xcursor
			${CMAKE_THREAD_LIBS_INIT}
			${CMAKE_DL_LIBS}
	)
elseif(WIN32)
	target_link_libraries(QuarkCore
			${CMAKE_CURRENT_SOURCE_DIR}/libs/libglfw3/win64/glfw3.lib
			${CMAKE_CURRENT_SOURCE_DIR}/libs/libglfw3/win32/glfw3.lib
			opengl32
			ws2_32
	)
endif()

#Build Quark
project (Quark C CXX)
set(CMAKE_CXX_STANDARD 11)
add_executable(Quark src/main.cpp)
target_link_libraries(Quark QuarkCore)
if(WIN32)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Quark)
endif()

#Build C/C++ Plugin
project (CPP_Plugin C CXX)
add_library(CPP_Plugin SHARED
		src/cpp_plugin/CPP_Plugin.cpp
		src/cpp_plugin/build.cpp
		src/ParsingUtils.cpp
		src/parson.cpp
)
target_link_libraries(CPP_Plugin QuarkCore)
add_custom_command(TARGET CPP_Plugin POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CPP_Plugin> $<TARGET_FILE_DIR:Quark>/plugins)
add_custom_command(TARGET CPP_Plugin POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove $<TARGET_FILE:CPP_Plugin>)

#Build Python Plugin
project (Python_Plugin C CXX)
add_library(Python_Plugin SHARED
		src/python_plugin/PythonPlugin.cpp
		src/ParsingUtils.cpp
)
target_link_libraries(Python_Plugin QuarkCore)
add_custom_command(TARGET Python_Plugin POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Python_Plugin> $<TARGET_FILE_DIR:Quark>/plugins)
add_custom_command(TARGET Python_Plugin POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove $<TARGET_FILE:Python_Plugin>)

#Build tests
project (Tests C CXX)
add_executable(Test src/tests/main.cpp)

#This is just so the solution is named correctly
project (Quark)
