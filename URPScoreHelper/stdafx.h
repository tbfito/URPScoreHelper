// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "targetver.h"
#include <cstdio>
#include <cstdlib>
#include <winsock.h>
#include <ctime>
#include <iostream>
#include <string>
#include "libfcgi/fcgio.h"

extern "C"
{
#ifdef _WIN64
	#pragma comment(lib, "ws2_32.lib")
	#include "curl-7.53.1-win64-mingw/include/curl/curl.h"
	#include "sqlite-3.1.8-win64/sqlite3.h"
	#pragma comment(lib, "libfcgi/libfcgi_x64.lib")
	#pragma comment(lib, "sqlite-3.1.8-win64/sqlite3.lib")
	#pragma comment(lib, "curl-7.53.1-win64-mingw/bin/libcurl.lib")
#else
	#ifdef _WIN32
		#pragma comment(lib, "ws2_32.lib")
		#include "curl-7.53.1-win32-mingw/include/curl/curl.h"
		#include "sqlite-3.1.8-win32/sqlite3.h"
		#pragma comment(lib, "libfcgi/libfcgi.lib")
		#pragma comment(lib, "sqlite-3.1.8-win32/sqlite3.lib")
		#pragma comment(lib, "curl-7.53.1-win32-mingw/bin/libcurl.lib")
	#endif
#endif
}