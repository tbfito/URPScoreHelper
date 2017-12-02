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
#include "libmysql/mysql.h"
#include "curl/curl.h"

using namespace std;

extern "C"
{
	#ifdef WIN32
		#include "resource.h"
	#else
		#include <unistd.h>
	#endif

	#ifdef _WIN64
		#pragma comment(lib, "libfcgi/libfcgi_win64.lib")
		#pragma comment(lib, "libmysql/libmysql_win64.lib")
		#pragma comment(lib, "curl/libcurl_win64.lib")
	#else
		#ifdef _WIN32
			#pragma comment(lib, "libfcgi/libfcgi_win32.lib")
			#pragma comment(lib, "sqlite/libmysql_win32.lib")
			#pragma comment(lib, "curl/libcurl_win32.lib")
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
#define MAX_PATH 4096