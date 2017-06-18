#pragma once
#ifndef URPSCOREHELPER_H
#define URPSCOREHELPER_H

const char *CGI_SCRIPT_NAME; // 脚本名字
const char *CGI_REQUEST_METHOD; // 请求方法
const char *CGI_CONTENT_LENGTH; // 数据长度
const char *CGI_QUERY_STRING; // 查询参数
const char *CGI_PATH_TRANSLATED; // 脚本位置
const char *CGI_HTTP_COOKIE; // Cookie
const char *CGI_SERVER_SOFTWARE; // 服务器软件
char *header;
char *footer;
char *error;

FILE *g_fQueryCount;
INT64 g_QueryCount;
clock_t g_start_time;
int g_users;

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
bool LoadPageSrc();
void OAuth2_Association(bool isPOST);
void LoadConfig();
void parse_teaching_evaluation();
void teaching_evaluation();
void parse_change_password();
void do_change_password();
void parse_ajax_captcha();

typedef struct test_info
{
	char kcmz[256] = {0};
	char cj[64] = {0};
	char lb[64] = {0};
	int date = 0;
} test_info;

typedef struct teach_eval
{
	char wjbm[64] = { 0 };
	char bpr[64] = { 0 };
	char pgnr[64] = { 0 };
	char name[256] = { 0 };
	char wjmc[1024] = { 0 };
	char bprm[1024] = { 0 };
	char pgnrm[1024] = { 0 };
	bool evaled = false;
} teach_eval;

#endif