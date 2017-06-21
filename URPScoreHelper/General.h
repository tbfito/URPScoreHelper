#pragma once

#define SOFTWARE_NAME "唯扬小助手"
#ifdef ZeroMemory
#undef ZeroMemory
#endif
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))

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
extern const char* REQUEST_PHOTO;
extern const char* REQUEST_TOP;
extern const char* GET_GRADE_BY_QBINFO;
extern const char* GET_GRADE_BY_PLAN;
extern const char* GET_GRADE_BY_FAILED;
extern const char* GET_SMALL_TEST_SCORE;
extern const char* GET_TEACH_EVAL_LIST;
extern const char* POST_TEACH_EVAL;
extern const char* POST_PRE_TEACH_EVAL;
extern const char* REQ_CHANGE_PASSWORD;

extern char *SERVER;
extern char *OAUTH2_APPID;
extern char *OAUTH2_SECRET;

extern char* ERROR_HTML;
extern char JSESSIONID[256];
extern void Error(char *p_ErrMsg);
extern char * base64_encode(const unsigned char * bindata, char * base64, int binlength);
float cj2jd(float cj);