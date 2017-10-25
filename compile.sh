#!/bin/bash
cd URPScoreHelper
sudo apt-get install libfcgi-dev libfcgi curl libcurl4-openssl-dev libsqlite3-dev
gcc -o URPScoreHelper -O2 -std=c++11 stdafx.cpp gbkutf8.cpp General.cpp CCurlTask.cpp StringHelper.cpp Encrypt.cpp Admin.cpp OAuth2.cpp URPRequests.cpp URPScoreHelper.cpp main.cpp -lstdc++ -lcurl -lfcgi++ -lfcgi -lsqlite3