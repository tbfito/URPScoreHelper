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
	extern char *CGI_X_FORWARDED_PROTO; // Upstream protocol

	extern char *HEADER_TEMPLATE_LOCATION;
	extern char *FOOTER_TEMPLATE_LOCATION;

	extern char GLOBAL_HEADER[256];
	extern char GLOBAL_HEADER_NO_CACHE_PLAIN_TEXT[512];
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
	extern const char *find_user_result_section;

	extern MYSQL db;
	extern char MYSQL_HOST[64];
	extern char MYSQL_PORT_NUMBER[64];
	extern char MYSQL_USERNAME[64];
	extern char MYSQL_PASSWORD[64];
	extern char MYSQL_DBNAME[128];

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
	extern bool CGI_X_IS_AJAX_REQUEST;

	extern char JSESSIONID[1024];

	void Error(const char *p_ErrMsg);
	char * base64_encode(const unsigned char * bindata, char * base64, int binlength);
	bool base64_decode(const char *strIn, unsigned char *pOut, size_t *uOutLen);
	std::string ReadTextFileToMem(const char *lpszLocalPath);
	std::string strformat(const char *format, ...);
	float cj2jd(float cj);
	std::string getAppURL();

#endif