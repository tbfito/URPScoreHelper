#pragma once

#ifndef __URPSCOREHELPER_H__

	#define __URPSCOREHELPER_H__

	std::string header;
	std::string footer;
	std::string error;

	int g_QueryCounter = 0;
	int g_users = 0;

	MYSQL db;

	FCGX_Request request;
	static const char *emptystr = "";

	char *dbConnError = NULL;
	char *CGI_SCRIPT_NAME = NULL;
	char *CGI_REQUEST_URI = NULL;
	char *CGI_REQUEST_METHOD = NULL;
	char *CGI_CONTENT_LENGTH = NULL;
	char *CGI_QUERY_STRING = NULL;
	char *CGI_SCRIPT_FILENAME = NULL;
	char *CGI_HTTP_COOKIE = NULL;
	char *CGI_HTTP_HOST = NULL;
	char *CGI_HTTPS = NULL;
	char *CGI_X_FORWARDED_PROTO = NULL;
	bool CGI_X_IS_AJAX_REQUEST = false;

	bool isPageSrcLoadSuccess = false;
	bool isdbReady = false;

	void fastcgi_app_intro();
	void END_REQUEST();
	void parse_main();
	int process_cookie(bool *p_need_update_cookie, std::string & p_photo_uri);
	void parse_index();
	void parse_query();
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
	void UpdateCounter();
	void parse_ajax_avatar();
	void SetQueryCounter(int current_counts);
	bool GetSettings(const char *name, char *value);
	bool AddSettings(const char *name, const char *value);
	void parse_query_tests();
	void parse_discussion();

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
		char name[1024] = { 0 };
		char wjmc[1024] = { 0 };
		char bprm[1024] = { 0 };
		char pgnrm[1024] = { 0 };
		bool evaled = false;
	} teach_eval;

#endif