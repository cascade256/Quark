#pragma once

//Cross platform exporting
#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#elif __linux__
#define EXPORT extern "C"
#endif

//Simple function types
typedef void(*Func)();
typedef void(*FuncWithArgs)(void*);
