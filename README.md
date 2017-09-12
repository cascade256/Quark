# Quark
 A simple and lightweight code editor

### **Warning!!!**
Quark is still new and very untested. It may lose work, crash, and who knows what else. 
Do not open important files with it, and backup files often.

## Features
* Quick startup, file loading, and file saving
* Syntax highlighting
* Plugin system 
  * C/C++ plugin with a simple build system
  * Python plugin
* Multithreaded job system to keep the UI quick and responsive
* A find function 
#### On the roadmap:
* Autocompletion
* Settings
	* Themes
	* Font selection
	* Tab settings
* Plugin based debugger
* Code navigation/refactoring tools
	* Go to definition
	* Go to declaration
	* Find all references
	* Automatic formatting
	* etc
* Improved find/replace

## Building

Quark uses CMake as it's build system. To build, simply run CMake to configure and run make or Visual Studio. 


## Resources used

* Nuklear <https://github.com/vurtun/nuklear>
* GLFW <http://www.glfw.org/>
* Code New Roman <https://sourceforge.net/projects/codenewroman/>
* Droid Sans Mono <https://fonts.google.com/specimen/Droid+Sans+Mono>
* Droid Sans <https://fonts.google.com/specimen/Droid+Sans>

Thank you to all the authors of and contributors to these projects, they have been a joy to use.
