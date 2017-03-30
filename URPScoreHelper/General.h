#pragma once

#define SERVER_IP "58.220.248.249"
//#define SERVER_IP "10.191.0.24"

// 公开给外部使用的变量
extern SOCKET g_so;

extern const char* GLOBAL_HEADER;
extern const char* SCORE_TEMPLATE;
extern const char* QUICK_SCORE;

extern const char* REQUEST_HOME_PAGE;
extern const char* REQUEST_HOME_PAGE_WITH_COOKIE;
extern const char* REQUEST_CAPTCHA;
extern const char* REQUEST_LOGIN;
extern const char* REQUEST_QUERY_SCORE;
extern const char* REQUEST_LOGOUT;
extern const char* REQUEST_GET_REGISTER_INTERFACE;
extern const char* REQUEST_POST_REGISTER_INTERFACE;
extern const char* REQUEST_SET_REPORT_PARAMS;
extern const char* REQUEST_REPORT_FILES;
extern const char* REQUEST_TXT_SCORES;

extern char* ERROR_HTML;
extern char JSESSIONID[256];
extern void Error(char *p_ErrMsg);
extern char * base64_encode(const unsigned char * bindata, char * base64, int binlength);