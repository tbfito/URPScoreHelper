#pragma once

#ifndef __GENERAL_H__

	#define __GENERAL_H__

	#define SOFTWARE_NAME "iEdon URPScoreHelper"
	#define SOFTWARE_COPYRIGHT u8"Copyright (C) 2012-2018 iEdon"


	// 从 URPScoreHelper.h 中引入并一并继承给引用本文件的文件

	extern char *CGI_SCRIPT_NAME; // 脚本名字
	extern char *CGI_REQUEST_URI; // 请求URI
	extern char *CGI_REQUEST_METHOD; // 请求方法
	extern char *CGI_CONTENT_LENGTH; // 数据长度
	extern char *CGI_QUERY_STRING; // 查询参数
	extern char *CGI_SCRIPT_FILENAME; // 脚本位置
	extern char *CGI_HTTP_COOKIE; // Cookie
	extern char *CGI_HTTP_HOST; // 请求主机
	extern char *CGI_HTTPS; // HTTPS
	extern char *CGI_HTTP_X_FORWARDED_PROTO; // Upstream protocol
	extern char *CGI_HTTP_FORWARDED; // 2014 年 RFC7239 标准化

	extern char *HEADER_TEMPLATE_LOCATION;
	extern char *FOOTER_TEMPLATE_LOCATION;

	extern char APP_SUB_DIRECTORY[MAX_PATH];
	extern char GLOBAL_HEADER[1024];
	extern char GLOBAL_HEADER_TYPE_PLAIN_TEXT[1024];
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
	extern const char *TEST_LIST_HTML;
	extern const char *FIND_USER_RESULT_SECTION;
	extern const char *LOGGED_USER_HTML;

	extern MYSQL db;
	extern char MYSQL_HOST[1024];
	extern char MYSQL_PORT_NUMBER[1024];
	extern char MYSQL_USERNAME[1024];
	extern char MYSQL_PASSWORD[1024];
	extern char MYSQL_DBNAME[1024];

	extern int CURL_CONN_TIMEOUT;
	extern bool CURL_USE_PROXY;
	extern char *SERVER_URL;
	extern char *USER_AGENT;
	extern char *CURL_PROXY_URL;
	extern char *APP_NAME;
	extern bool ENABLE_OAUTH2;
	extern char *OAUTH2_APPID;
	extern char *OAUTH2_SECRET;
	extern char *CARD_AD_BANNER_1_IMG;
	extern char *CARD_AD_BANNER_2_IMG;
	extern char *CARD_AD_BANNER_1_URL;
	extern char *CARD_AD_BANNER_2_URL;
	extern bool ENABLE_QUICK_QUERY;
	extern char *ADMIN_USER_NAME;
	extern char *ADMIN_PASSWORD;
	extern char *SECONDARY_TITLE;
	extern char *APP_KEYWORDS;
	extern char *APP_DESCRIPTION;
	extern char *FOOTER_TEXT;
	extern char *ANALYSIS_CODE;
	extern char *HOMEPAGE_NOTICE;
	extern char *DISCUSSION_PAGE_CONTENT;
	extern char *DISCUSSION_PAGE_CODE;
	extern char *SITE_MAINTENANCE;

	extern std::string ERROR_HTML;
	extern std::string error;
	extern bool CGI_HTTP_X_IS_AJAX_REQUEST;

	extern char JSESSIONID[1024];

	void Error(const char *p_ErrMsg);
	float cj2jd(float cj);
	std::string getAppURL();
	void output_token_header(const char *m_xuehao, const char *m_password);
	std::string generate_token(const char *m_xuehao, const char *m_password);
	void decode_token(char *token, std::string & token_xh, std::string & token_mm);

#endif