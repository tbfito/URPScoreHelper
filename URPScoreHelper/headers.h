#pragma once

#ifndef __HEADERS_H__

#define __HEADERS_H__

	#define _CRT_SECURE_NO_WARNINGS

	#include <stdio.h>
	#include <stdlib.h>
	#include <time.h>
	#include <string.h>
	#include <malloc.h>
	#include <iostream>
	#include <string>
	#include "libfcgi/fcgio.h"
	#include "libmysql/mysql.h"
	#include "curl/curl.h"

	using std::cout;
	using std::cin;
	using std::cerr;
	using std::string;

	extern "C"
	{
		#ifdef WIN32 // Windows 平台
			#include "resource.h"
		#else // Unix Like
			#include <unistd.h>
		#endif

		#ifdef _WIN64 // Windows amd64(x86-64) 平台
			#pragma comment(lib, "libfcgi/libfcgi_win64.lib")
			#pragma comment(lib, "libmysql/libmysql_win64.lib")
			#pragma comment(lib, "curl/libcurl_win64.lib")
		#else
			#ifdef _WIN32 // Windows x86 平台
				#pragma comment(lib, "libfcgi/libfcgi_win32.lib")
				#pragma comment(lib, "libmysql/libmysql_win32.lib")
				#pragma comment(lib, "curl/libcurl_win32.lib")
			#endif
		#endif
	}

	// 宏重定义
	#ifdef MAX_PATH
		#undef MAX_PATH
	#endif
	#define MAX_PATH 4096

#endif