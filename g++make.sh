#!/bin/bash
cd URPScoreHelper
sudo apt-get install libfcgi-dev libfcgi curl libcurl4-openssl-dev libsqlite3-dev
g++ -o URPScoreHelper.exe -O2 -std=c++11 -fpermissive -Woverflow -Wwrite-strings stdafx.cpp gbkutf8.cpp General.cpp CCurlTask.cpp StringHelper.cpp Encrypt.cpp ini.cpp INIReader.cpp OAuth2.cpp URPRequests.cpp URPScoreHelper.cpp -lstdc++ -lcurl -lfcgi++ -lfcgi -lsqlite3