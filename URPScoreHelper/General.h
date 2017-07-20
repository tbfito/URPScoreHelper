#pragma once

#define SOFTWARE_NAME "iEdon URPScoreHelper"
#define SOFTWARE_COPYRIGHT "Copyright (C) 2012-2017 iEdon Inside"


// �ⲿʹ�õı���

extern char *CGI_SCRIPT_NAME; // �ű�����
extern char *CGI_REQUEST_URI; // ����URI
extern char *CGI_REQUEST_METHOD; // ���󷽷�
extern char *CGI_CONTENT_LENGTH; // ���ݳ���
extern char *CGI_QUERY_STRING; // ��ѯ����
extern char *CGI_SCRIPT_FILENAME; // �ű�λ��
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

extern sqlite3 * db;
extern int CURL_TIMEOUT;
extern bool CURL_USE_PROXY;
extern char *SERVER_URL;
extern char *USER_AGENT;
extern char *CURL_PROXY_URL;
extern char *APP_NAME;
extern char *OAUTH2_APPID;
extern char *OAUTH2_SECRET;
extern std::string ERROR_HTML;

extern char JSESSIONID[256];

void Error(const char *p_ErrMsg);
char * base64_encode(const unsigned char * bindata, char * base64, int binlength);
std::string ReadTextFileToMem(const char *lpszLocalPath);
std::string strformat(const char *format, ...);
float cj2jd(float cj);
std::string getAppURL();