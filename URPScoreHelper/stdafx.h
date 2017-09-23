// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#include <string.h>
#include <malloc.h>
#include <iostream>
#include <string>
#include "libfcgi/fcgio.h"
#include "sqlite/sqlite3.h"
#include "curl/curl.h"

using namespace std;

extern "C"
{
#ifdef _WIN64
	#include "resource.h"
	#pragma comment(lib, "libfcgi/libfcgi_win64.lib")
	#pragma comment(lib, "sqlite/sqlite3_win64.lib")
	#pragma comment(lib, "curl/libcurl_win64.lib")
#else
	#ifdef _WIN32
		#include "resource.h"
		#pragma comment(lib, "libfcgi/libfcgi_win32.lib")
		#pragma comment(lib, "sqlite/sqlite3_win32.lib")
		#pragma comment(lib, "curl/libcurl_win32.lib")
	#else
		#include <unistd.h>
	#endif
#endif
}

// Unix 兼容 Win 宏定义
#ifdef ZeroMemory
	#undef ZeroMemory
#endif
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#ifdef MAX_PATH
	#undef MAX_PATH
#endif
#define MAX_PATH 260

/*
#define _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC_NEW
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
*/
//#include <vld.h>
