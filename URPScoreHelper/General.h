#pragma once

#define SOFTWARE_NAME "唯扬小助手"
#define SOFTWARE_COPYRIGHT "Copyright (C) 2012-2017 iEdon Inside"


// 外部使用的变量

extern char *CGI_SCRIPT_NAME; // 脚本名字
extern char *CGI_REQUEST_URI; // 请求URI
extern char *CGI_REQUEST_METHOD; // 请求方法
extern char *CGI_CONTENT_LENGTH; // 数据长度
extern char *CGI_QUERY_STRING; // 查询参数
extern char *CGI_SCRIPT_FILENAME; // 脚本位置
extern char *CGI_HTTP_COOKIE; // Cookie
extern char *CGI_HTTP_HOST;
extern char *CGI_HTTPS;

extern const char* GLOBAL_HEADER;
extern const char* SCORE_TEMPLATE;
extern const char* BEFORE_TEMPLATE;
extern const char* AFTER_TEMPLATE;
extern const char* QUICK_SCORE;

extern sqlite3 * db;
extern int CURL_TIMEOUT;
extern bool CURL_USE_PROXY;
extern char *SERVER_URL;
extern char *USER_AGENT;
extern char *CURL_PROXY_URL;
extern char *OAUTH2_APPID;
extern char *OAUTH2_SECRET;
extern std::string ERROR_HTML;

extern char JSESSIONID[256];

void Error(const char *p_ErrMsg);
char * base64_encode(const unsigned char * bindata, char * base64, int binlength);
std::string ReadTextFileToMem(const char *lpszLocalPath);
std::string strformat(const char *format, ...);
float cj2jd(float cj);