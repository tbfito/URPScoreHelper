#pragma once
#ifndef URPSCOREHELPER_H
#define URPSCOREHELPER_H

std::string header;
std::string footer;
std::string error;

FILE *g_fQueryCount;
unsigned long long g_QueryCount;
clock_t g_start_time;
int g_users;
sqlite3 *db;

FCGX_Request request; //ȫ�ֿ���ʹ�õ�����
char *CGI_SCRIPT_NAME; // �ű�����
char *CGI_REQUEST_URI; // ����URI
char *CGI_REQUEST_METHOD; // ���󷽷�
char *CGI_CONTENT_LENGTH; // ���ݳ���
char *CGI_QUERY_STRING; // ��ѯ����
char *CGI_SCRIPT_FILENAME; // �ű�λ��
char *CGI_HTTP_COOKIE; // Cookie
char *CGI_HTTP_HOST;
char *CGI_HTTPS;
bool isPageSrcLoadSuccess;
bool isdbReady;

int parse_main(bool p_need_set_cookie, std::string & p_photo);
int process_cookie(bool *p_need_update_cookie, std::string & p_photo_uri);
int parse_index();
int parse_query();
void parse_friendly_score(std::string & p_strlpszScore);
void get_student_name(char *p_lpszBuffer);
void get_student_id(char *p_lpszBuffer);
int system_registration();
void student_logout();
bool student_login(char *p_xuehao, char *p_password, char *p_captcha);
void parse_QuickQuery_Intro();
void parse_QuickQuery_Result();
void LoadPageSrc();
void OAuth2_Association(bool isPOST);
void LoadConfig();
void parse_teaching_evaluation();
void teaching_evaluation();
void parse_change_password();
void do_change_password();
void parse_ajax_captcha();
void SetUsersCounter();

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