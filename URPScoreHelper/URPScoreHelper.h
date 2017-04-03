#pragma once
const char *CGI_REQUEST_METHOD; // 请求方法
const char *CGI_CONTENT_LENGTH; // 数据长度
const char *CGI_SCRIPT_NAME; // 脚本名称
const char *CGI_QUERY_STRING; // 查询参数
const char *CGI_PATH_TRANSLATED; // 脚本位置
const char *CGI_HTTP_COOKIE; // Cookie
const char *CGI_SERVER_SOFTWARE; // 服务器软件

FILE *g_fQueryCount;
INT64 g_QueryCount;
clock_t g_start_time;

int parse_main(bool p_need_set_cookie, char *p_photo, bool p_is_login);
int process_cookie(bool *p_need_update_cookie, char *p_photo_uri);
int parse_index();
int parse_query();
void parse_friendly_score(char *p_lpszScore);
void get_student_name(char *p_lpszBuffer);
void get_student_id(char *p_lpszBuffer);
int system_registration();
void student_logout();
bool student_login(char *p_xuehao, char *p_password, char *p_captcha);
void parse_QuickQuery_Intro();
void parse_QuickQuery_Result();

typedef struct test_info
{
	char kcmz[256] = {0};
	char cj[64] = {0};
	char lb[64] = {0};
	int date = 0;
};