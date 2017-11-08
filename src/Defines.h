#pragma once

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#elif __linux__
#define EXPORT 
#endif

typedef void(*Func)();//A basic void to void function pointer