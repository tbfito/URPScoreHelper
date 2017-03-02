// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO:  在此处引用程序需要的其他头文件

#include <stdlib.h>
#include <Windows.h>
#include <winsock.h>
#include <time.h>
#include <string.h>
#include "libfcgi_include/fcgio.h"  
#include "libfcgi_include/fcgi_config.h"
#include <iostream>
#pragma comment(lib, "libfcgi.lib")
#pragma comment(lib, "ws2_32.lib")
