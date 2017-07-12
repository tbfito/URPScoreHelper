#!/bin/bash
cd URPScoreHelper
g++ -o URPScoreHelper.exe -O2 -std=c++11 -fpermissive -Woverflow -Wwrite-strings stdafx.cpp General.cpp CCurlTask.cpp StringHelper.cpp Encrypt.cpp ini.cpp INIReader.cpp OAuth2.cpp URPRequests.cpp URPScoreHelper.cpp -L "../Unix_libs" -lcurl -lfcgi -lsqlite3