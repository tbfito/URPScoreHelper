#!/bin/bash
cd URPScoreHelper
apt-get install libfcgi-dev libfcgi curl libcurl4-openssl-dev
gcc -o URPScoreHelper.exe -O2 -std=c++11 headers.cpp gbkutf8.cpp General.cpp CCurlTask.cpp StringHelper.cpp Encrypt.cpp Admin.cpp OAuth2.cpp URPRequests.cpp URPScoreHelper.cpp AES.cpp INIReader.cpp ini.cpp main.cpp -lstdc++ -lcurl -lfcgi++ -lfcgi -lm -lpthread -ldl -lmysqlclient -L "./libmysql"
strip URPScoreHelper.exe
mv URPScoreHelper.exe ../URPScoreHelper.exe