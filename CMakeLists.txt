cmake_minimum_required(VERSION 3.5)

#Build Quark
project (Quark C CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(Quark src/FileTree.cpp src/FindDialog.cpp src/gl3w.cpp src/hashmap.cpp src/JobManager.cpp src/LayoutManager.cpp src/Logger.cpp src/main.cpp src/MainMenu.cpp src/OpenFileManager.cpp src/ParsingUtils.cpp src/PluginManager.cpp src/TextEditor.cpp src/tinycthread.c src/nuklear/nuklear.cpp)

if(UNIX)
    find_package(Threads)
    target_link_libraries(Quark glfw3 GL X11 Xrandr Xinerama Xxf86vm Xcursor ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
elseif(WIN32)
    message("Windows!")
endif()


#Copy font
file(COPY src/DroidSansMono.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

#Create plugins folder
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plugins)

#Build C/C++ Plugin
project (CPP_Plugin C CXX)
add_library(CPP_Plugin SHARED src/cpp_plugin/CPP_Plugin.cpp src/cpp_plugin/build.cpp src/ParsingUtils.cpp src/parson.cpp)

#Build Python Plugin
project (Python_Plugin C CXX)
add_library(Python_Plugin SHARED src/python_plugin/PythonPlugin.cpp src/ParsingUtils.cpp)

#Set the plugins output folder
set_target_properties(CPP_Plugin Python_Plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")
