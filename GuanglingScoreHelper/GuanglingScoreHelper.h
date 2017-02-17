#pragma once
char *CGI_REQUEST_METHOD; // 请求方法
char *CGI_CONTENT_LENGTH; // 数据长度
char *CGI_SCRIPT_NAME; // 脚本名称
char *CGI_QUERY_STRING; // 查询参数
char *CGI_PATH_TRANSLATED; // 脚本位置
char *CGI_HTTP_COOKIE; // Cookie

FILE *g_fQueryCount;
INT64 g_QueryCount;

int parse_index();
int parse_query();
void parse_friendly_score(char *p_lpszScore);
void get_student_name(char *p_lpszBuffer);
int system_registration();