#pragma once

#define SOFTWARE_NAME "iEdon-URPScoreHelper"
#define SOFTWARE_COPYRIGHT "Copyright © 2012-2017 iEdon Inside"


// 外部使用的变量

extern char *CGI_SCRIPT_NAME; // 脚本名字
extern char *CGI_REQUEST_URI; // 请求URI
extern char *CGI_REQUEST_METHOD; // 请求方法
extern char *CGI_CONTENT_LENGTH; // 数据长度
extern char *CGI_QUERY_STRING; // 查询参数
extern char *CGI_SCRIPT_FILENAME; // 脚本位置
extern char *CGI_HTTP_COOKIE; // Cookie
extern char *CGI_HTTP_HOST; // Host
extern char *CGI_HTTPS; // is secure HTTP

extern const char *GLOBAL_HEADER;
extern const char *SCORE_TEMPLATE;
extern const char *BEFORE_TEMPLATE;
extern const char *AFTER_TEMPLATE;
extern const char *BEFORE_TEMPLATE_BY_PLAN;
extern const char *SCORE_TEMPLATE_BY_PLAN;
extern const char *QUICK_SCORE;
extern const char *OAUTH2_LOGIN_HTML;
extern const char *QUICKQUERY_HTML;
extern const char *ASSOC_LINK_HTML;
extern const char *RLS_ASSOC_LINK_HTML;
extern const char *CARD_AD_BANNER_HTML;

extern sqlite3 * db;
extern int CURL_TIMEOUT;
extern bool CURL_USE_PROXY;
extern char *SERVER_URL;
extern char *USER_AGENT;
extern char *CURL_PROXY_URL;
extern char *APP_NAME;
extern char *OAUTH2_APPID;
extern char *OAUTH2_SECRET;
extern char *CARD_AD_BANNER_1_IMG;
extern char *CARD_AD_BANNER_2_IMG;
extern char *CARD_AD_BANNER_1_URL;
extern char *CARD_AD_BANNER_2_URL;
extern std::string ERROR_HTML;

extern char JSESSIONID[256];

void Error(const char *p_ErrMsg);
char * base64_encode(const unsigned char * bindata, char * base64, int binlength);
std::string ReadTextFileToMem(const char *lpszLocalPath);
std::string strformat(const char *format, ...);
float cj2jd(float cj);
std::string getAppURL();