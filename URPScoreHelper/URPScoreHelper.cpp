/*
******************************************
********** iEdon URPScoreHelper **********
**********  Model + Controller  **********
**********   Copyright © iEdon  **********
******************************************
****  Project encoding must be UTF-8  ****
******************************************
*/

#include "headers.h"
#include "URPScoreHelper.h"
#include "General.h"
#include "StringHelper.h"
#include "CCurlTask.h"
#include "OAuth2.h"
#include "URPRequests.h"
#include "Encrypt.h"
#include "gbkutf8.h"
#include "Admin.h"

// 请求映射入口 (主控制器：FastCGI 处理循环)
void fastcgi_app_intro()
{
	while (FCGX_Accept_r(&request) >= 0)
	{
		LoadConfig(); // 再次更新配置信息
		if (isdbReady)
		{
			UpdateCounter();
		}

		fcgi_streambuf cin_fcgi_streambuf(request.in);
		fcgi_streambuf cout_fcgi_streambuf(request.out);
		fcgi_streambuf cerr_fcgi_streambuf(request.err);

		cin.rdbuf(&cin_fcgi_streambuf);
		cout.rdbuf(&cout_fcgi_streambuf);
		cerr.rdbuf(&cerr_fcgi_streambuf);

		CGI_REQUEST_URI = FCGX_GetParam("REQUEST_URI", request.envp); // 请求URI
		CGI_REQUEST_METHOD = FCGX_GetParam("REQUEST_METHOD", request.envp); // 请求方法
		CGI_CONTENT_LENGTH = FCGX_GetParam("CONTENT_LENGTH", request.envp); // 数据长度
		CGI_SCRIPT_NAME = FCGX_GetParam("SCRIPT_NAME", request.envp); // 脚本名称
		CGI_QUERY_STRING = FCGX_GetParam("QUERY_STRING", request.envp); // 查询参数
		CGI_SCRIPT_FILENAME = FCGX_GetParam("SCRIPT_FILENAME", request.envp); // 脚本位置
		CGI_HTTP_COOKIE = FCGX_GetParam("HTTP_COOKIE", request.envp); // Cookie
		CGI_HTTPS = FCGX_GetParam("HTTPS", request.envp); // 是否使用 HTTPS
		CGI_HTTP_HOST = FCGX_GetParam("HTTP_HOST", request.envp); // 请求主机
		CGI_HTTP_X_FORWARDED_PROTO = FCGX_GetParam("HTTP_X_FORWARDED_PROTO", request.envp); // 上游代理与客户端所使用的协议
		CGI_HTTP_FORWARDED = FCGX_GetParam("HTTP_FORWARDED", request.envp); // 上游代理传过来的客户端信息 (2014 RFC7239)
		CGI_HTTP_X_IS_AJAX_REQUEST = (FCGX_GetParam("HTTP_X_AJAX_REQUEST", request.envp) != NULL); // 是否是 AJAX 请求(自行约定)
		char *XRW = FCGX_GetParam("HTTP_X_REQUESTED_WITH", request.envp); // 是否是 AJAX 请求(通过XHR判定)
		if (!CGI_HTTP_X_IS_AJAX_REQUEST && XRW != NULL)
		{
			std::string str_XRW(XRW);
			std::transform(str_XRW.begin(), str_XRW.end(), str_XRW.begin(), tolower);
			if (str_XRW == "xmlhttprequest")
				CGI_HTTP_X_IS_AJAX_REQUEST = true;
		}

		if (!isdbReady)
		{
			std::cout << "Status: 500 Internal Server Error\r\n"
				<< GLOBAL_HEADER
				<< u8"<h1>数据库连接失败</h1><p>" << dbConnError << "</p>";
			END_REQUEST(); continue;
		}

		if (CGI_REQUEST_METHOD == NULL)
		{
			CGI_REQUEST_METHOD = (char *)emptystr;
		}
		if (CGI_SCRIPT_NAME == NULL)
		{
			CGI_SCRIPT_NAME = (char *)emptystr;
		}
		if (CGI_QUERY_STRING == NULL)
		{
			CGI_QUERY_STRING = (char *)emptystr;
		}
		if (CGI_SCRIPT_FILENAME == NULL)
		{
			CGI_SCRIPT_FILENAME = (char *)emptystr;
		}
		if (CGI_CONTENT_LENGTH == NULL)
		{
			CGI_CONTENT_LENGTH = (char *)emptystr;
		}
		if (CGI_HTTP_COOKIE == NULL)
		{
			CGI_HTTP_COOKIE = (char *)emptystr;
		}
		if (CGI_HTTP_HOST == NULL)
		{
			CGI_HTTP_HOST = (char *)emptystr;
		}

		size_t sub_direcotry_offset = strlen(APP_SUB_DIRECTORY);
		if (sub_direcotry_offset != 0) {
			if (strstr(CGI_SCRIPT_NAME, APP_SUB_DIRECTORY) != NULL) {
				CGI_SCRIPT_NAME += sub_direcotry_offset;
			}
			if (CGI_REQUEST_URI != NULL && strstr(CGI_REQUEST_URI, APP_SUB_DIRECTORY) != NULL)
			{
				CGI_REQUEST_URI += sub_direcotry_offset;
			}
		}

		// 单独为 Admin 做处理，将其请求转发到 admin.cpp 控制器。
		if (strlen(CGI_REQUEST_URI) >= 6)
		{
			char left_str[128] = { 0 };
			left(left_str, CGI_REQUEST_URI, 6);
			if (strcmp(left_str, "/admin") == 0)
			{
				admin_intro();
				END_REQUEST(); continue;
			}
		}

		if (!isPageSrcLoadSuccess)
		{
			LoadPageSrc();
			if (!isPageSrcLoadSuccess)
			{
				std::cout << "Status: 500 Internal Server Error\r\n"
					<< GLOBAL_HEADER
					<< u8"<p>网页模板文件缺失或异常</p>";
				END_REQUEST(); continue;
			}
		}

		// 为第三方接入做名称转换
		std::string str_qs_3rd_party = _GET(std::string(CGI_QUERY_STRING), "3rd_party");
		if (!str_qs_3rd_party.empty())
		{
			char _3rd_party[4096] = { 0 };
			strncpy(_3rd_party, str_qs_3rd_party.c_str(), sizeof(_3rd_party) - 1);
			std::cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << CGI_SCRIPT_NAME << "\r\n";
			std::cout << "Set-Cookie: 3rd_party=" << _3rd_party << "; max-age=1800; path=" << APP_SUB_DIRECTORY << "/\r\n";
			std::cout << GLOBAL_HEADER;
			END_REQUEST(); continue;
		}
		std::string str_c_3rd_party = _COOKIE(std::string(CGI_HTTP_COOKIE), "3rd_party");
		if (!str_c_3rd_party.empty())
		{
			char _3rd_party[4096] = { 0 };
			strncpy(_3rd_party, str_c_3rd_party.c_str(), sizeof(_3rd_party) - 1);
			url_decode(_3rd_party, str_c_3rd_party.length());
			memset(APP_NAME, 0, 10240);
			strncpy(APP_NAME, _3rd_party, sizeof(_3rd_party) - 1);
		}

		// 普通请求处理
		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0) // 如果是 GET 请求
		{
			if (strcmp(CGI_SCRIPT_NAME, "") == 0)
			{
				std::cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/") == 0 || strcmp(CGI_SCRIPT_NAME, "/index.fcgi") == 0)
			{
				if (_GET(std::string(CGI_QUERY_STRING), "act") == "logout")
				{
					student_logout();
					std::cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
					END_REQUEST(); continue;
				}
				if (_GET(std::string(CGI_QUERY_STRING), "act") == "linking")
				{
					bool m_need_update_cookie = false;
					std::string photo(" ");
					process_cookie(&m_need_update_cookie, photo);
					if (photo.empty())
					{
						std::cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
						END_REQUEST(); continue;
					}
					char student_id[512] = { 0 };
					get_student_id(student_id);
					student_logout();
					EnCodeStr(student_id, student_id);
					std::cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/OAuth2.fcgi?user=" << student_id << "\r\n" << GLOBAL_HEADER;
					END_REQUEST(); continue;
				}
				if (strcmp(CGI_REQUEST_URI, "/index.fcgi") == 0)
				{
					std::cout << "Status: 301 Moved Permanently\r\n" << "Location: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
					END_REQUEST(); continue;
				}
				parse_index();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/main.fcgi") == 0)
			{
				parse_main();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2.fcgi") == 0)
			{
				if (_GET(std::string(CGI_QUERY_STRING), "act") == "callback")
				{
					OAuth2_callback();
					END_REQUEST(); continue;
				}
				if (_GET(std::string(CGI_QUERY_STRING), "act") == "link")
				{
					OAuth2_linking(false);
					END_REQUEST(); continue;
				}
				OAuth2_process();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/query.fcgi") == 0)
			{
				parse_query();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/QuickQuery.fcgi") == 0)
			{
				parse_QuickQuery_Intro();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/TeachEval.fcgi") == 0)
			{
				parse_teaching_evaluation();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/changePassword.fcgi") == 0)
			{
				parse_change_password();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/captcha.fcgi") == 0)
			{
				parse_ajax_captcha();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/avatar.fcgi") == 0)
			{
				parse_ajax_avatar();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/discussion.fcgi") == 0)
			{
				parse_discussion();
				END_REQUEST(); continue;
			}
			std::cout << "Status: 404 Not Found\r\n";
			Error(u8"<p>找不到该页面</p>");
			END_REQUEST(); continue;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0) // 如果是 POST 请求
		{
			if (strcmp(CGI_SCRIPT_NAME, "") == 0)
			{
				std::cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/changePassword.fcgi") == 0)
			{
				do_change_password();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/query.fcgi") == 0)
			{
				if (_GET(std::string(CGI_QUERY_STRING), "act") == "QuickQuery")
				{
					parse_QuickQuery_Result();
					END_REQUEST(); continue;
				}
				if (_GET(std::string(CGI_QUERY_STRING), "order") == "tests")
				{
					parse_query_tests();
					END_REQUEST(); continue;
				}
				parse_query();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/main.fcgi") == 0)
			{
				parse_main();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2.fcgi") == 0)
			{
				if (_GET(std::string(CGI_QUERY_STRING), "act") == "link")
				{
					OAuth2_linking(true);
					END_REQUEST(); continue;
				}
				OAuth2_process();
				END_REQUEST(); continue;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/TeachEval.fcgi") == 0)
			{
				if (_GET(std::string(CGI_QUERY_STRING), "act") == "Evaluate")
				{
					teaching_evaluation();
					END_REQUEST(); continue;
				}
			}
		}
		std::cout << "Status: 405 Method Not Allowed\r\n";
		Error(u8"<p>发生错误，请求的方法不被允许</p>");
		END_REQUEST(); continue;
	}
}

// 结束请求
void END_REQUEST()
{
	memset(JSESSIONID, 0, sizeof(JSESSIONID));
	FCGX_Finish_r(&request);
}

// 预加载头部和尾部页面(header.fcgi, footer.fcgi, error.fcgi)
void LoadPageSrc()
{
	// 读入主页面文件
	char *pStr = strstr(CGI_SCRIPT_FILENAME, "\\");
	bool isUnixBasedPath = (pStr == NULL);
	if (isUnixBasedPath)
	{
		pStr = strstr(CGI_SCRIPT_FILENAME, "/");
	}
	if (pStr == NULL)
	{
		isPageSrcLoadSuccess = false;
		return;
	}
	char *Last = NULL;
	while (pStr != NULL)
	{
		Last = pStr;
		if (isUnixBasedPath)
		{
			pStr = strstr(pStr + 1, "/");
		}
		else
		{
			pStr = strstr(pStr + 1, "\\");
		}
	}
	char *doc_root = new char[MAX_PATH];
	memset(doc_root, 0, MAX_PATH);

	mid(doc_root, CGI_SCRIPT_FILENAME, Last - CGI_SCRIPT_FILENAME + 1, 0);
	char *file_root = new char[MAX_PATH];

	memset(file_root, 0, MAX_PATH);
	strncpy(file_root, doc_root, MAX_PATH - 1);
	strcat(file_root, "header.fcgi");
	strncpy(HEADER_TEMPLATE_LOCATION, file_root, MAX_PATH - 1); // HEADER_TEMPLATE_LOCATION 内存在 LoadConfig() 中做了初始化，内存大小 MAX_PATH 已清零

	header = strformat(ReadTextFileToMem(file_root).c_str(), "%s", SECONDARY_TITLE, APP_KEYWORDS, APP_DESCRIPTION);

	memset(file_root, 0, MAX_PATH);
	strncpy(file_root, doc_root, MAX_PATH - 1);
	strcat(file_root, "footer.fcgi");
	strncpy(FOOTER_TEMPLATE_LOCATION, file_root, MAX_PATH - 1); // FOOTER_TEMPLATE_LOCATION 内存在 LoadConfig() 中做了初始化，内存大小 MAX_PATH 已清零

	footer = strformat(ReadTextFileToMem(file_root).c_str(), APP_NAME, FOOTER_TEXT, ANALYSIS_CODE);

	memset(file_root, 0, MAX_PATH);
	strncpy(file_root, doc_root, MAX_PATH - 1);
	strcat(file_root, "error.fcgi");
	
	error = ReadTextFileToMem(file_root);

	// 未能加载这些模板
	if (header.empty() || footer.empty() || error.empty())
	{
		isPageSrcLoadSuccess = false;
		delete[]doc_root;
		delete[]file_root;
		return;
	}
	
	std::string title(u8"提示 - ");
	title.append(APP_NAME);
	ERROR_HTML = strformat(header.c_str(), title.c_str());
	ERROR_HTML += error + footer;

	delete[]doc_root;
	delete[]file_root;
	isPageSrcLoadSuccess = true;
}

// 加载配置
void LoadConfig()
{
	if (HEADER_TEMPLATE_LOCATION == NULL)
	{
		HEADER_TEMPLATE_LOCATION = (char *)malloc(MAX_PATH);
		memset(HEADER_TEMPLATE_LOCATION, 0, MAX_PATH);
	}
	if (FOOTER_TEMPLATE_LOCATION == NULL)
	{
		FOOTER_TEMPLATE_LOCATION = (char *)malloc(MAX_PATH);
		memset(FOOTER_TEMPLATE_LOCATION, 0, MAX_PATH);
	}

	bool need_db_connection = false;
	if (!isdbReady)
	{
		need_db_connection = true;
	}
	else
	{
		if (mysql_ping(&db) != 0)
		{
			isdbReady = false;
			mysql_close(&db);
			memset(&db, 0, sizeof(MYSQL));
			mysql_init(&db);
			need_db_connection = true;
		}
	}
	if (need_db_connection)
	{
		if (!mysql_real_connect(&db, MYSQL_HOST, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DBNAME, atoi(MYSQL_PORT_NUMBER), NULL, 0))
		{
			isdbReady = false;
			fprintf(stderr, "Database Error: %s\n", mysql_error(&db));
			if (dbConnError != NULL)
			{
				free(dbConnError);
				dbConnError = NULL;
			}
			size_t errlen = strlen(mysql_error(&db));
			dbConnError = (char *)malloc(errlen + 1);
			memset(dbConnError, 0, errlen + 1);
			strncpy(dbConnError, mysql_error(&db), errlen);
			mysql_close(&db);
			memset(&db, 0, sizeof(MYSQL));
			mysql_init(&db);
			return;
		}
		if (dbConnError != NULL)
		{
			free(dbConnError);
			dbConnError = NULL;
		}

		// 设置允许数据库断开重连
		char value = 1;
		mysql_options(&db, MYSQL_OPT_RECONNECT, &value);

		isdbReady = true;

		std::string query("CREATE TABLE IF NOT EXISTS `userinfo` (`id` varchar(36) NOT NULL,`password` varchar(1024) NOT NULL,`name` varchar(36) DEFAULT NULL,`openid` varchar(1024) DEFAULT NULL,`OAuth_name` varchar(1024) DEFAULT NULL,`OAuth_avatar` varchar(4096) DEFAULT NULL,`lastlogin` datetime DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8");
		if (mysql_query(&db, query.c_str()) != 0)
		{
			return;
		}

		query = "CREATE TABLE IF NOT EXISTS `settings` (`name` varchar(254) NOT NULL,`value` varchar(10240) NOT NULL,PRIMARY KEY (`name`)) ENGINE=InnoDB DEFAULT CHARSET=utf8";
		if (mysql_query(&db, query.c_str()) != 0)
		{
			return;
		}

		query = "SET NAMES UTF8";
		mysql_query(&db, query.c_str());

		// 如果数据库没有下面配置，则自动增加并写入默认值以确保首次能够正常运行。
		AddSettings("QueryCounter", "0");
		AddSettings("SERVER_URL", "http://0.0.0.0");
		AddSettings("USER_AGENT", SOFTWARE_NAME);
		AddSettings("ENABLE_OAUTH2", "0");
		AddSettings("OAUTH2_APPID", "");
		AddSettings("OAUTH2_SECRET", "");
		AddSettings("CURL_PROXY_URL", "");
		AddSettings("APP_NAME", SOFTWARE_NAME);
		AddSettings("CURL_CONN_TIMEOUT", "3");
		AddSettings("CURL_USE_PROXY", "0");
		AddSettings("CARD_AD_BANNER_1_IMG", "");
		AddSettings("CARD_AD_BANNER_2_IMG", "");
		AddSettings("CARD_AD_BANNER_1_URL", "");
		AddSettings("CARD_AD_BANNER_2_URL", "");
		AddSettings("ADMIN_USER_NAME", "admin");
		AddSettings("ADMIN_PASSWORD", "admin");
		AddSettings("SECONDARY_TITLE", SOFTWARE_NAME);
		AddSettings("APP_KEYWORDS", "");
		AddSettings("APP_DESCRIPTION", "");
		AddSettings("FOOTER_TEXT", SOFTWARE_NAME);
		AddSettings("ANALYSIS_CODE", "");
		AddSettings("ENABLE_QUICK_QUERY", "0");
		AddSettings("HOMEPAGE_NOTICE", "");
		AddSettings("DISCUSSION_PAGE_CONTENT", "");
		AddSettings("DISCUSSION_PAGE_CODE", "");
		AddSettings("SITE_MAINTENANCE", "");
	}

	if (SERVER_URL != NULL)
	{
		free(SERVER_URL);
	}
	SERVER_URL = (char *)malloc(10240);
	if (USER_AGENT != NULL)
	{
		free(USER_AGENT);
	}
	USER_AGENT = (char *)malloc(10240);
	if (OAUTH2_APPID != NULL)
	{
		free(OAUTH2_APPID);
	}
	OAUTH2_APPID = (char *)malloc(10240);
	if (OAUTH2_SECRET != NULL)
	{
		free(OAUTH2_SECRET);
	}
	OAUTH2_SECRET = (char *)malloc(10240);
	if (CURL_PROXY_URL != NULL)
	{
		free(CURL_PROXY_URL);
	}
	CURL_PROXY_URL = (char *)malloc(10240);
	if (APP_NAME != NULL)
	{
		free(APP_NAME);
	}
	APP_NAME = (char *)malloc(10240);
	if (CARD_AD_BANNER_1_IMG != NULL)
	{
		free(CARD_AD_BANNER_1_IMG);
	}
	CARD_AD_BANNER_1_IMG = (char *)malloc(10240);
	if (CARD_AD_BANNER_2_IMG != NULL)
	{
		free(CARD_AD_BANNER_2_IMG);
	}
	CARD_AD_BANNER_2_IMG = (char *)malloc(10240);
	if (CARD_AD_BANNER_1_URL != NULL)
	{
		free(CARD_AD_BANNER_1_URL);
	}
	CARD_AD_BANNER_1_URL = (char *)malloc(10240);
	if (CARD_AD_BANNER_2_URL != NULL)
	{
		free(CARD_AD_BANNER_2_URL);
	}
	CARD_AD_BANNER_2_URL = (char *)malloc(10240);
	if (ADMIN_USER_NAME != NULL)
	{
		free(ADMIN_USER_NAME);
	}
	ADMIN_USER_NAME = (char *)malloc(10240);
	if (ADMIN_PASSWORD != NULL)
	{
		free(ADMIN_PASSWORD);
	}
	ADMIN_PASSWORD = (char *)malloc(10240);
	if (SECONDARY_TITLE != NULL)
	{
		free(SECONDARY_TITLE);
	}
	SECONDARY_TITLE = (char *)malloc(10240);
	if (APP_KEYWORDS != NULL)
	{
		free(APP_KEYWORDS);
	}
	APP_KEYWORDS = (char *)malloc(10240);
	if (APP_DESCRIPTION != NULL)
	{
		free(APP_DESCRIPTION);
	}
	APP_DESCRIPTION = (char *)malloc(10240);
	if (FOOTER_TEXT != NULL)
	{
		free(FOOTER_TEXT);
	}
	FOOTER_TEXT = (char *)malloc(10240);
	if (ANALYSIS_CODE != NULL)
	{
		free(ANALYSIS_CODE);
	}
	ANALYSIS_CODE = (char *)malloc(10240);
	if (HOMEPAGE_NOTICE != NULL)
	{
		free(HOMEPAGE_NOTICE);
	}
	HOMEPAGE_NOTICE = (char *)malloc(10240);
	if (DISCUSSION_PAGE_CONTENT != NULL)
	{
		free(DISCUSSION_PAGE_CONTENT);
	}
	DISCUSSION_PAGE_CONTENT = (char *)malloc(10240);
	if (DISCUSSION_PAGE_CODE != NULL)
	{
		free(DISCUSSION_PAGE_CODE);
	}
	DISCUSSION_PAGE_CODE = (char *)malloc(10240);
	if (SITE_MAINTENANCE != NULL)
	{
		free(SITE_MAINTENANCE);
	}
	SITE_MAINTENANCE = (char *)malloc(10240);

	char *lpvBuffer = (char *)malloc(10240);

	memset(SERVER_URL, 0, 10240);
	memset(USER_AGENT, 0, 10240);
	memset(OAUTH2_APPID, 0, 10240);
	memset(OAUTH2_SECRET, 0, 10240);
	memset(CURL_PROXY_URL, 0, 10240);
	memset(APP_NAME, 0, 10240);
	memset(lpvBuffer, 0, 10240);
	memset(CARD_AD_BANNER_1_IMG, 0, 10240);
	memset(CARD_AD_BANNER_2_IMG, 0 ,10240);
	memset(CARD_AD_BANNER_1_URL, 0, 10240);
	memset(CARD_AD_BANNER_2_URL, 0, 10240);
	memset(ADMIN_USER_NAME, 0, 10240);
	memset(ADMIN_PASSWORD, 0, 10240);
	memset(SECONDARY_TITLE, 0, 10240);
	memset(APP_KEYWORDS, 0, 10240);
	memset(APP_DESCRIPTION, 0, 10240);
	memset(FOOTER_TEXT, 0, 10240);
	memset(ANALYSIS_CODE, 0, 10240);
	memset(HOMEPAGE_NOTICE, 0, 10240);
	memset(DISCUSSION_PAGE_CONTENT, 0, 10240);
	memset(DISCUSSION_PAGE_CODE, 0, 10240);
	memset(SITE_MAINTENANCE, 0, 10240);

	GetSettings("SERVER_URL", SERVER_URL);
	GetSettings("USER_AGENT", USER_AGENT);
	GetSettings("OAUTH2_APPID", OAUTH2_APPID);
	GetSettings("OAUTH2_SECRET", OAUTH2_SECRET);
	GetSettings("CURL_PROXY_URL", CURL_PROXY_URL);
	GetSettings("APP_NAME", APP_NAME);
	GetSettings("CARD_AD_BANNER_1_IMG", CARD_AD_BANNER_1_IMG);
	GetSettings("CARD_AD_BANNER_2_IMG", CARD_AD_BANNER_2_IMG);
	GetSettings("CARD_AD_BANNER_1_URL", CARD_AD_BANNER_1_URL);
	GetSettings("CARD_AD_BANNER_2_URL", CARD_AD_BANNER_2_URL);
	GetSettings("ADMIN_USER_NAME", ADMIN_USER_NAME);
	GetSettings("ADMIN_PASSWORD", ADMIN_PASSWORD);
	GetSettings("SECONDARY_TITLE", SECONDARY_TITLE);
	GetSettings("APP_KEYWORDS", APP_KEYWORDS);
	GetSettings("APP_DESCRIPTION", APP_DESCRIPTION);
	GetSettings("FOOTER_TEXT", FOOTER_TEXT);
	GetSettings("ANALYSIS_CODE", ANALYSIS_CODE);
	GetSettings("HOMEPAGE_NOTICE", HOMEPAGE_NOTICE);
	GetSettings("DISCUSSION_PAGE_CONTENT", DISCUSSION_PAGE_CONTENT);
	GetSettings("DISCUSSION_PAGE_CODE", DISCUSSION_PAGE_CODE);
	GetSettings("SITE_MAINTENANCE", SITE_MAINTENANCE);

	GetSettings("CURL_CONN_TIMEOUT", lpvBuffer);
	CURL_CONN_TIMEOUT = atoi(lpvBuffer);
	if (CURL_CONN_TIMEOUT <= 0)
		CURL_CONN_TIMEOUT = 3;
	memset(lpvBuffer, 0, 10240);

	GetSettings("CURL_USE_PROXY", lpvBuffer);
	CURL_USE_PROXY = (atoi(lpvBuffer) >= 1);

	memset(lpvBuffer, 0, 10240);
	GetSettings("ENABLE_QUICK_QUERY", lpvBuffer);
	ENABLE_QUICK_QUERY = (atoi(lpvBuffer) >= 1);

	memset(lpvBuffer, 0, 10240);
	GetSettings("ENABLE_OAUTH2", lpvBuffer);
	ENABLE_OAUTH2 = (atoi(lpvBuffer) >= 1);

	free(lpvBuffer);

	if (FOOTER_TEMPLATE_LOCATION != NULL) // 每次更新 footer 的缓存
	{
		footer = strformat(ReadTextFileToMem(FOOTER_TEMPLATE_LOCATION).c_str(), APP_NAME, FOOTER_TEXT, ANALYSIS_CODE);
	}
}

// 更新用户数量、查询计数器
void UpdateCounter()
{
	g_QueryCounter = 0;
	g_users = 0;

	// 获取多少用户使用了我们的服务 :)
	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));
	std::string query("SELECT `value` FROM `settings` WHERE `name`='QueryCounter'");

	if (stmt == NULL)
	{
		return;
	}
	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()))
	{
		mysql_stmt_close(stmt);
		return;
	}

	char query_counts[64] = { 0 };
	
	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)query_counts;
	bind[0].buffer_length = sizeof(query_counts);

	if (mysql_stmt_bind_result(stmt, bind))
	{
		mysql_stmt_close(stmt);
		return;
	}
	if (mysql_stmt_execute(stmt))
	{
		mysql_stmt_close(stmt);
		return;
	}
	if (mysql_stmt_store_result(stmt))
	{
		mysql_stmt_close(stmt);
		return;
	}

	while (mysql_stmt_fetch(stmt) == 0)
		g_QueryCounter = atoi(query_counts);

	mysql_stmt_close(stmt);
	
	stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind2[1];
	memset(bind2, 0, sizeof(bind2));
	query = "SELECT COUNT(*) FROM `userinfo`";

	if (stmt == NULL)
	{
		return;
	}
	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()))
	{
		mysql_stmt_close(stmt);
		return;
	}

	bind2[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind2[0].buffer = (void *)query_counts;
	bind2[0].buffer_length = sizeof(query_counts);

	if (mysql_stmt_bind_result(stmt, bind))
	{
		mysql_stmt_close(stmt);
		return;
	}
	if (mysql_stmt_execute(stmt))
	{
		mysql_stmt_close(stmt);
		return;
	}
	if (mysql_stmt_store_result(stmt))
	{
		mysql_stmt_close(stmt);
		return;
	}

	while (mysql_stmt_fetch(stmt) == 0)
		g_users = atoi(query_counts);

	mysql_stmt_close(stmt);
}

// 读取数据库设置表中的内容(注意value的内存分配 10240B)
bool GetSettings(const char *name, char *value)
{
	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[1];
	MYSQL_BIND ret[1];
	memset(bind, 0, sizeof(bind));
	memset(ret, 0, sizeof(ret));
	std::string query("SELECT `value` FROM `settings` WHERE `name`=?");

	if (stmt == NULL)
	{
		return false;
	}

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)name;
	bind[0].buffer_length = strlen(name);

	char query_ret[10240] = { 0 };
	ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	ret[0].buffer = (void *)query_ret;
	ret[0].buffer_length = sizeof(query_ret);

	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
		mysql_stmt_bind_param(stmt, bind) != 0 || 
		mysql_stmt_bind_result(stmt, ret) != 0 || 
		mysql_stmt_execute(stmt) != 0 || 
		mysql_stmt_store_result(stmt) != 0)
	{
		mysql_stmt_close(stmt);
		strncpy(value, "", 1);
		return false;
	}

	while (mysql_stmt_fetch(stmt) == 0);

	if (strlen(query_ret) == 0)
	{
		mysql_stmt_close(stmt);
		strncpy(value, "", 1);
		return false;
	}

	mysql_stmt_close(stmt);
	strncpy(value, query_ret, 10240 - 1);
	return true;
}

// 向数据库设置表中增添配置项
bool AddSettings(const char *name, const char *value)
{
	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[2];
	memset(bind, 0, sizeof(bind));
	std::string query("INSERT IGNORE INTO `settings` (`name`, `value`) VALUES (?, ?)");

	if (stmt == NULL)
	{
		return false;
	}
	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()))
	{
		mysql_stmt_close(stmt);
		return false;
	}

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)name;
	bind[0].buffer_length = strlen(name);

	bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[1].buffer = (void *)value;
	bind[1].buffer_length = strlen(value);

	if (mysql_stmt_bind_param(stmt, bind))
	{
		mysql_stmt_close(stmt);
		return false;
	}

	if (mysql_stmt_execute(stmt))
	{
		mysql_stmt_close(stmt);
		return false;
	}
	mysql_stmt_close(stmt);
	return true;
}

// 置查询计数器
void SetQueryCounter(int current_counts)
{
	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[1];
	memset(bind, 0, sizeof(bind));
	std::string query("UPDATE `settings` SET `value`=? WHERE `name`='QueryCounter'");

	if (stmt == NULL)
	{
		return;
	}
	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()))
	{
		mysql_stmt_close(stmt);
		return;
	}

	std::string counts = strformat("%d", current_counts);
	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)counts.c_str();
	bind[0].buffer_length = counts.length();

	mysql_stmt_bind_param(stmt, bind);
	mysql_stmt_execute(stmt);
	mysql_stmt_close(stmt);
}

// 处理 Cookie、照片(p_photo_uri 为空代表不要照片, 随便设置内容不为空则会向里面写入照片数据)
int process_cookie(bool *p_need_update_cookie, std::string & p_photo_uri)
{
	std::string str_JSESSIONID = _COOKIE(std::string(CGI_HTTP_COOKIE), "JSESSIONID");
	if (!str_JSESSIONID.empty()) // 如果客户端已经拿到 JSESSIONID，看看原 Cookie 是否过期、有效（即服务器是否设置了新 Cookie）
	{
		CCurlTask req;
		if (!req.Exec(true, REQUEST_HOME_PAGE, CGI_HTTP_COOKIE))
		{
			p_photo_uri.erase();
			return -1;
		}
		
		std::string str_newJSESSIONID = _COOKIE(req.GetResultString(), "JSESSIONID");
		if (!str_newJSESSIONID.empty())
		{
			strncpy(JSESSIONID, str_newJSESSIONID.c_str(), sizeof(JSESSIONID) - 1);
			*p_need_update_cookie = true;
			return -1;
		}
		else // 如果 Cookie 还能用，就获取它。
		{
			strncpy(JSESSIONID, str_JSESSIONID.c_str(), sizeof(JSESSIONID) - 1);
		}
	}
	else
	{
		CCurlTask req;
		if (!req.Exec(true, REQUEST_HOME_PAGE))
		{
			p_photo_uri.erase();
			return -1;
		}

		// 获取 Session ID。
		str_JSESSIONID = _COOKIE(req.GetResultString(), "JSESSIONID");
		if (str_JSESSIONID.empty())
		{
			p_photo_uri.erase();
			return -1;
		}

		strncpy(JSESSIONID, str_JSESSIONID.c_str(), sizeof(JSESSIONID) - 1); // 成功获得 Session ID。
		*p_need_update_cookie = true;
	}

	if (p_photo_uri.empty())
		return -1;  // p_photo_uri 指定了 NULL 代表不需要照片。

	// 看看登录没
	std::string tmpStr ("JSESSIONID=");
	tmpStr += JSESSIONID;

	CCurlTask req;
	if (!req.Exec(false, REQUEST_PHOTO, tmpStr))
	{
		p_photo_uri.erase();
		return -1;
	}
	char *m_photo = req.GetResult();
	if (strstr(m_photo, "\xb5\xc7\xc2\xbc" /*登录*/) == NULL)
	{
		size_t m_photoLength = req.GetLength();
		char *m_base64 = (char *)malloc(m_photoLength * 3 + 1);
		memset(m_base64, 0, m_photoLength * 3 + 1);

		base64_encode((const unsigned char *)m_photo, m_base64, m_photoLength);

		size_t m_PhotoDataURI_len = m_photoLength * 3 + 1 + 24;
		char *m_PhotoDataURI = (char *)malloc(m_PhotoDataURI_len);
		memset(m_PhotoDataURI, 0, m_PhotoDataURI_len);

		strncpy(m_PhotoDataURI, "data:image/jpg;base64,", m_PhotoDataURI_len - 1);
		strcat(m_PhotoDataURI, m_base64);
		p_photo_uri = m_PhotoDataURI;

		free(m_base64);
		free(m_PhotoDataURI);
	}
	else
	{
		p_photo_uri.erase();
	}
	return 1;
}

// 处理 /main.fcgi
void parse_main()
{
	if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
	{
		// 获取 POST 数据。
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0 || m_post_length > 127)
		{
			Error(u8"<p>输入的帐号或密码有问题，请重试</p>");
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
		std::string post(m_post_data);
		free(m_post_data);

		// 获取学号
		std::string str_xuehao = _POST(post, "xh");
		if (str_xuehao.empty())
		{
			Error(u8"<p>无法获取学号信息</p>");
			return;
		}
		char m_xuehao[128] = { 0 };
		strncpy(m_xuehao, str_xuehao.c_str(), sizeof(m_xuehao) - 1);

		// 获取密码
		std::string str_password = _POST(post, "mm");
		if (str_password.empty())
		{
			Error(u8"<p>无法获取密码信息</p>");
			return;
		}
		char m_password[4096] = { 0 };
		strncpy(m_password, str_password.c_str(), sizeof(m_password) - 1);

		// 获取验证码
		std::string str_captcha = _POST(post, "yzm");
		if (str_captcha.empty())
		{
			Error(u8"<p>无法获取验证码信息</p>");
			return;
		}
		char m_captcha[128] = { 0 };
		strncpy(m_captcha, str_captcha.c_str(), sizeof(m_captcha) - 1);

		if (!student_login(m_xuehao, m_password, m_captcha))
		{
			return;
		}

		output_token_header(m_xuehao, m_password);
	}

	// 读入主页面文件
	std::string m_lpszHomepage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	char m_student_name[128] = {0};
	char m_student_id[512] = { 0 };
	char m_avatar_url[MAX_PATH] = { 0 };
	get_student_id(m_student_id);
	if (!GetOAuthUserInfo(m_student_id, m_student_name, m_avatar_url, sizeof(m_student_name), sizeof(m_avatar_url)))
	{
		memset(m_student_name, 0, sizeof(m_student_name));
		get_student_name(m_student_name);
	}

	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[1];
	MYSQL_BIND ret[1];
	memset(bind, 0, sizeof(bind));
	memset(ret, 0, sizeof(ret));
	std::string query("SELECT `openid` FROM `userinfo` WHERE `id`=?");

	char openid[1024] = { 0 };
	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)m_student_id;
	bind[0].buffer_length = strlen(m_student_id);

	ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	ret[0].buffer = (void *)openid;
	ret[0].buffer_length = sizeof(openid);

	if (stmt != NULL)
	{
		mysql_stmt_prepare(stmt, query.c_str(), query.length());
		mysql_stmt_bind_param(stmt, bind);
		mysql_stmt_bind_result(stmt, ret);
		mysql_stmt_execute(stmt);
		mysql_stmt_store_result(stmt);
		while (mysql_stmt_fetch(stmt) == 0);
		mysql_stmt_close(stmt);
	}

	cout << GLOBAL_HEADER;

	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		std::string title(m_student_name);
		title += u8" - ";
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	std::string OutputAd;
	if (strlen(CARD_AD_BANNER_1_IMG) > 0)
	{
		OutputAd += strformat(CARD_AD_BANNER_HTML, CARD_AD_BANNER_1_URL, CARD_AD_BANNER_1_IMG);
	}
	if (strlen(CARD_AD_BANNER_2_IMG) > 0)
	{
		OutputAd += strformat(CARD_AD_BANNER_HTML, CARD_AD_BANNER_2_URL, CARD_AD_BANNER_2_IMG);
	}
	if (strlen(openid) == 0)
	{
		cout << strformat(m_lpszHomepage.c_str(), APP_NAME, m_student_name, m_student_id, 
							ENABLE_OAUTH2 ? ASSOC_LINK_HTML : "", OutputAd.c_str());
	}
	else
	{
		cout << strformat(m_lpszHomepage.c_str(), APP_NAME, m_student_name, m_student_id,
							ENABLE_OAUTH2 ? strformat(RLS_ASSOC_LINK_HTML, m_student_id).c_str() : "", OutputAd.c_str());
	}
	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << footer.c_str();
	}
}

// 处理主页面请求 (GET / /index.fcgi)
void parse_index()
{
	if (strlen(SITE_MAINTENANCE) != 0)
	{
		Error(SITE_MAINTENANCE);
		return;
	}

	std::string wx_xh;
	std::string wx_mm;
	std::string token_xh;
	std::string token_mm;
	std::string str_token = _GET(std::string(CGI_QUERY_STRING), "token");
	if (!str_token.empty()) // 如果是微信登录回来，QS中有token，则自动填充帐号密码。
	{
		char token[4096] = { 0 };
		strncpy(token, str_token.c_str(), sizeof(token) - 1);
		decode_token(token, wx_xh, wx_mm);
	}
	else // 如果COOKIE中有token，则自动填充帐号密码。
	{
		char token[4096] = { 0 };
		std::string str_token = _COOKIE(std::string(CGI_HTTP_COOKIE), "token");
		if (!str_token.empty())
		{
			strncpy(token, str_token.c_str(), sizeof(token) - 1);
			decode_token(token, token_xh, token_mm);
		}
	}

	// 读入主页面文件
	std::string m_lpszHomepage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	cout << GLOBAL_HEADER;
	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << strformat(header.c_str(), APP_NAME);
	}

	bool hasNotice = (strlen(HOMEPAGE_NOTICE) != 0);
	std::string notice("<div id=\"i_notice\">");
	if (hasNotice)
	{
		notice = notice + HOMEPAGE_NOTICE;
	}
	notice = notice + "</div>";

	if (wx_xh.empty() || wx_mm.empty())
	{
		if (token_xh.empty() == false && token_mm.empty() == false)
		{
			cout << strformat(m_lpszHomepage.c_str(), APP_NAME, u8"教务系统登录", hasNotice ? notice.c_str() : "", "",
								token_xh.c_str(), token_mm.c_str(), ENABLE_OAUTH2 ? " col-50" : "", u8"登录",
								ENABLE_OAUTH2 ? OAUTH2_LOGIN_HTML : "", ENABLE_QUICK_QUERY ? QUICKQUERY_HTML : "");
		}
		else
		{
			cout << strformat(m_lpszHomepage.c_str(), APP_NAME, u8"教务系统登录", hasNotice ? notice.c_str() : "", "",
								"", "", ENABLE_OAUTH2 ? " col-50" : "", u8"登录",
								ENABLE_OAUTH2 ? OAUTH2_LOGIN_HTML : "", ENABLE_QUICK_QUERY ? QUICKQUERY_HTML : "");
		}
	}
	else
	{
		char m_OAuth_name[512];
		char m_OAuth_avatar[MAX_PATH];
		GetOAuthUserInfo((char *)wx_xh.c_str(), m_OAuth_name, m_OAuth_avatar, sizeof(m_OAuth_name), sizeof(m_OAuth_avatar));

		cout << strformat(m_lpszHomepage.c_str(), APP_NAME, u8"微信快速登录", hasNotice ? notice.c_str() : "",
						  (strlen(m_OAuth_name) != 0 && strlen(m_OAuth_avatar) != 0) ? strformat(LOGGED_USER_HTML, m_OAuth_avatar, m_OAuth_name).c_str() : "",
						  wx_xh.c_str(), wx_mm.c_str(), "", u8"继续", "", "");

	}
	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << footer.c_str();
	}
}

// 处理验证码 Ajax 请求
void parse_ajax_captcha() //(AJAX: GET /captcha.fcgi)
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	if (process_cookie(&m_need_update_cookie, m_photo) == -1) {
		cout << GLOBAL_HEADER_TYPE_PLAIN_TEXT;
		cout << "REQUEST-FAILED";
		return;
	}

	if (m_need_update_cookie)
	{
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
	}

	if (!m_photo.empty() && !m_need_update_cookie) // 登录了就通报已经登录
	{
		cout << GLOBAL_HEADER_TYPE_PLAIN_TEXT;
		cout << "LOGGED-IN";
		return;
	}

	// 置随机数种子，并取得一个随机数，用于获取验证码。
	srand((int)time(NULL));
	int m_rand = rand();
	std::string Captcha = strformat(REQUEST_CAPTCHA, m_rand);

	// 发送验证码请求，获取验证码数据。
	CCurlTask req;
	std::string cookie;
	if (m_need_update_cookie)
	{
		cookie = "JSESSIONID=";
		cookie.append(JSESSIONID);
	}
	else
	{
		cookie = CGI_HTTP_COOKIE;
	}
	if (!req.Exec(false, Captcha, cookie))
	{
		cout << GLOBAL_HEADER_TYPE_PLAIN_TEXT;
		cout << "REQUEST-FAILED";
		return;
	}
	char *m_rep_body = req.GetResult();

	int m_CaptchaLength = req.GetLength(); // 验证码图片的大小

	// 将验证码转化为 base64 编码后的 DataURL，浏览器直接显示，供用户查看。
	char *m_base64 = new char[m_CaptchaLength * 2 + 1];
	base64_encode((const unsigned char *)m_rep_body, m_base64, m_CaptchaLength);
	char *m_DataURL = new char[m_CaptchaLength * 2 + 24];
	memset(m_DataURL, 0, m_CaptchaLength * 2 + 24);
	strncpy(m_DataURL, "data:image/jpg;base64,", m_CaptchaLength * 2 + 24 - 1);
	strcat(m_DataURL, m_base64);

	cout << GLOBAL_HEADER_TYPE_PLAIN_TEXT;
	cout << m_DataURL;

	delete[]m_base64;
	delete[]m_DataURL;
}

// 处理头像 Ajax 请求
void parse_ajax_avatar()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	if (process_cookie(&m_need_update_cookie, m_photo) == -1) {
		cout << GLOBAL_HEADER_TYPE_PLAIN_TEXT;
		cout << "LOGGED-OUT";
		return;
	}

	if (m_need_update_cookie)
	{
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
	}

	if (m_photo.empty() || m_need_update_cookie)
	{
		cout << GLOBAL_HEADER_TYPE_PLAIN_TEXT;
		cout << "LOGGED-OUT";
		return;
	}
	cout << GLOBAL_HEADER_TYPE_PLAIN_TEXT;
	char m_student_id[512] = { 0 };
	char m_avatar_url[MAX_PATH] = { 0 };
	char m_student_name[1024] = { 0 };
	get_student_id(m_student_id);
	if (!GetOAuthUserInfo(m_student_id, m_student_name, m_avatar_url, sizeof(m_student_name), sizeof(m_avatar_url)))
	{
		if (m_photo != "data:image/jpg;base64,") // 判断教务系统中有无头像，即 datauri 有没有值
		{
			cout << m_photo.c_str();
		}
		else
		{
			cout << APP_SUB_DIRECTORY << "/img/default_avatar.jpg";
		}
	}
	else
	{
		cout << m_avatar_url;
	}
}

// 处理查询页面请求 (GET /query.fcgi)
void parse_query()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // 还没登录就丢去登录。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
		return;
	}

	// 开始查分(本学期)。
	CCurlTask req;
	if (!req.Exec(false, REQUEST_QUERY_SCORE, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>接收数据失败</b></p><p>网络通信异常</p>");
		student_logout();
		return;
	}

	char m_Student[128] = { 0 };
	get_student_name(m_Student);

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	char *m_query_not_reg = strstr((char *)req.GetResultString().c_str(), "\xc3\xbb\xd3\xd0\xd7\xa2\xb2\xe1" /*没有注册*/);
	if (m_query_not_reg != NULL) // 如果还没有电子注册
	{
		if (system_registration() != 0)
		{
			return;
		}
	}
	if (_GET(std::string(CGI_QUERY_STRING), "order") == "tests")
	{
		CCurlTask req;
		if (!req.Exec(false, GET_TEST_LIST, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>网络通信异常</p>");
			return;
		}

		char *p1 = strstr(req.GetResult(), "] = new Array(\"");
		if (p1 == NULL)
		{
			Error(u8"<p><b>从服务器拉取分数失败 (BeginOfList)</b></p><p>可能现在还没出成绩</p>");
			return;
		}
		std::string script(u8"<script type=\"text/javascript\">function init_test_list() {$(\"#tests\").select({title:\"选择考试\",items:[");
		bool isFirst = true;
		while (p1 != NULL)
		{
			char *p2 = strstr(p1+16, "\");");
			if (p2 == NULL)
			{
				Error(u8"<p><b>从服务器拉取分数失败 (EndOfLine)</b></p><p>请稍后再试</p>");
				return;
			}
			char Line[4096] = { 0 };
			mid(Line, p1, p2 - p1 + 3, 0);
			char testName[4096] = { 0 };
			char testID[4096] = { 0 };
			char termID[4096] = { 0 };
			
			int matches = sscanf(Line, "%*[^\"]\"%[^\"]\",\"%[^\"]\",\"%[^\"]\");", testName, testID, termID);
			if (matches != 3)
			{
				Error(u8"<p><b>从服务器拉取分数失败 (MatchLineError)</b></p><p>请稍后再试</p>");
				return;
			}

			int lowYear = 0, highYear = 0, season = 0, season_part = 0;
			int termMatches = sscanf(termID, "%d-%d-%d-%d", &lowYear, &highYear, &season, &season_part);
			if (termMatches != 4)
			{
				Error(u8"<p><b>从服务器拉取分数失败 (MatchTermError)</b></p><p>请稍后再试</p>");
				return;
			}

			std::string term = strformat("[%d-%d%s] ", lowYear, highYear, season == 1 ? u8"秋" : u8"春");

			unsigned int u8len = strlen(testName) * 3 + 1;
			char *u8testName = (char *)malloc(u8len);
			gbk_to_utf8(testName, (unsigned int)strlen(testName), &u8testName, &u8len);
			term.append(u8testName);

			if (!isFirst)
			{
				script += ",";
			}
			else
			{
				isFirst = false;
			}
			script = script + "{title:\"" + term + "\",value:\"" + termID + "|" + testID + "\"}";
			free(u8testName);

			p1 = strstr(p2 + 3, "] = new Array(\"");
		}
		script = script + "]});}</script>" + TEST_LIST_HTML;
		cout << GLOBAL_HEADER;
		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			std::string title(m_Student);
			title += u8"的考试成绩 - ";
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat(m_lpszQuery.c_str(), m_Student, script.c_str());
		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			cout << footer.c_str();
		}
		return;
	}

	if (_GET(std::string(CGI_QUERY_STRING), "order") == "passed")
	{
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_QBINFO, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>网络通信异常</p>");
			return;
		}
		if (req.GetLength() <= 256) // 经过测试，当新生没有已过科目的时候，服务器返回空body的内容，大小211b.
		{
			Error(u8"<p>现在还没有已过科目的数据</p>");
			return;
		}

		unsigned int rep_len = req.GetLength() * 3 + 1;
		char *m_rep_body = (char *)malloc(rep_len);
		gbk_to_utf8(req.GetResult(), (unsigned int)req.GetLength(), &m_rep_body, &rep_len);
		req.GetResultString() = m_rep_body;
		free(m_rep_body);
		m_rep_body = req.GetResult();

		char *m_result = strstr(m_rep_body, "<body leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\" style=\"overflow:auto;\">");
		if (m_result == NULL)
		{
			Error(u8"<p><b>从服务器拉取分数失败 (BeginOfRet)</b></p><p>系统可能正忙，请稍后再试</p>");
			return;
		}
		m_result += 92;
		char *m_prep = (char *)malloc(req.GetLength());
		memset(m_prep, 0, req.GetLength());
		strncpy(m_prep, "<div id=\"list_page\">", req.GetLength() - 1);
		char *m_end_body = strstr(m_result, "</body>");
		if (m_end_body == NULL)
		{
			free(m_prep);
			Error(u8"<p>从服务器拉取分数失败 (EndOfBodyNotFound)</p>");
			return;
		}

		replace_string(m_result, "&nbsp;", "");
		replace_string(m_result, "\t", "");
		replace_string(m_result, "\r", "");
		replace_string(m_result, "\n", "");

		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			std::string title(m_Student);
			title += u8"的通过科目 - ";
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			cout << footer.c_str();
		}

		SetQueryCounter(++g_QueryCounter);

		free(m_prep);
		return;
	}

	if (_GET(std::string(CGI_QUERY_STRING), "order") == "byplan")
	{
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_PLAN, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>网络通信异常</p>");
			return;
		}
		
		char *m_rep_body = req.GetResult();

		char *m_result = strstr(m_rep_body, "<body leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\" style=\"overflow:auto;\">");
		if (m_result == NULL)
		{
			Error(u8"<p><b>从服务器拉取分数失败 (BeginOfRet)</b></p><p>系统可能正忙，请稍后再试</p>");
			return;
		}

		std::string m_Output;
		m_result += 92;

		char *p1 = strstr(m_result, "<td class=\"legend\">");
		if (p1 != NULL)
		{
			char *p2 = strstr(p1 + 20, "</td>");
			if (p2 != NULL)
			{
				char zymc[128] = { 0 };
				mid(zymc, p1 + 19, p2 - p1 - 19, 0);
				unsigned int u8len = strlen(zymc) * 3 + 1;
				char *u8strtmp = (char *)malloc(u8len);
				gbk_to_utf8(zymc, (unsigned int)strlen(zymc), &u8strtmp, &u8len);
				m_Output.append("<div id=\"i_total\"><p style=\"font-size: 16px\"><b>").append(u8strtmp).append(u8"</b>：</p>");
				free(u8strtmp);
				p1 = strstr(p2, "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" id=\"tblView\">");
				if (p1 != NULL)
				{
					p2 = strstr(p1 + 64, "</table>");
					if (p2 != NULL)
					{
						char info[1024] = { 0 };
						mid(info, p1, p2 - p1 + 9, 0);
						replace_string(info, " style=\"font-weight: bold\"", "");
						replace_string(info, "tblView", "byplan");
						unsigned int u8len = strlen(info) * 3 + 1;
						char *u8strtmp = (char *)malloc(u8len);
						gbk_to_utf8(info, (unsigned int)strlen(info), &u8strtmp, &u8len);
						m_Output.append(u8strtmp);
						free(u8strtmp);
					}
				}
				m_Output.append("</div>");
			}
		}

		m_Output.append(BEFORE_TEMPLATE_BY_PLAN);

		p1 = strstr(m_result, "<td align=\"center\">");
		bool isSuccess = true;
		enum byplan_item_flags { kechenghao , kexuhao, kechengming, yingwenkechengming, xuefen, shuxing, chengji, weiguoyuanyin};
		int flags = kechenghao;

		char m_kechenghao[128] = { 0 };
		char m_kexuhao[128] = { 0 };
		char m_kechengming[1024] = { 0 };
		char m_yingwenkechengming[1024] = { 0 };
		char m_xuefen[64] = { 0 };
		char m_shuxing[64] = { 0 };
		char m_chengji[64] = { 0 };
		char m_weiguoyuanyin[2048] = { 0 };
		float f_xuefen = 0.0;
		float f_chengji = 0.0;
		float f_jidian = 0.0;
		bool hasChengji = true;

		if (p1 == NULL)
		{
			hasChengji = false;
		}
		while (p1 != NULL)
		{
			char *p2 = NULL;
			if (flags == weiguoyuanyin || flags == chengji)
			{
				p2 = strstr(p1 + 19, "</P>");
			}
			else
			{
				p2 = strstr(p1 + 20, "</td>");
			}

			if (p2 == NULL)
			{
				isSuccess = false;
				break;
			}
			switch (flags) {
				case kechenghao:
				{
					mid(m_kechenghao, p1 + 20, p2 - p1 - 20, 0);
					////Trim(m_kechenghao);
					flags++;
					break;
				}
				case kexuhao:
				{
					mid(m_kexuhao, p1 + 20, p2 - p1 - 20, 0);
					//Trim(m_kexuhao);
					flags++;
					break;
				}
				case kechengming:
				{
					mid(m_kechengming, p1 + 20, p2 - p1 - 20, 0);
					//Trim(m_kechengming);
					flags++;
					break;
				}
				case yingwenkechengming:
				{
					mid(m_yingwenkechengming, p1 + 20, p2 - p1 - 20, 0);
					//Trim(m_yingwenkechengming);
					flags++;
					break;
				}
				case xuefen:
				{
					mid(m_xuefen, p1 + 20, p2 - p1 - 20, 0);
					//Trim(m_xuefen);
					f_xuefen = atof(m_xuefen);
					flags++;
					break;
				}
				case shuxing:
				{
					mid(m_shuxing, p1 + 20, p2 - p1 - 20, 0);
					//Trim(m_shuxing);
					flags++;
					break;
				}
				case chengji:
				{
					mid(m_chengji, p1, p2 - p1, 0);
					char *p3 = strstr(m_chengji, "<p align=\"center\">");
					if (p3 != NULL)
					{
						mid(m_chengji, p3 + 18, strlen(m_chengji), 0);
					}
					//Trim(m_chengji);
					f_chengji = atof(m_chengji);
					if (strstr(m_chengji, "\xd3\xc5\xd0\xe3" /*优秀*/) != NULL)
					{
						f_chengji = 95;
					}
					if (strstr(m_chengji, "\xc1\xbc\xba\xc3" /*良好*/) != NULL)
					{
						f_chengji = 85;
					}
					if (strstr(m_chengji, "\xd6\xd0\xb5\xc8" /*中等*/) != NULL)
					{
						f_chengji = 75;
					}
					if (strstr(m_chengji, "\xbc\xb0\xb8\xf1" /*及格*/) != NULL)
					{
						f_chengji = 60;
					}
					if (strstr(m_chengji, "\xb2\xbb\xbc\xb0\xb8\xf1" /*不及格*/) != NULL)
					{
						f_chengji = 55;
					}
					f_jidian = cj2jd(f_chengji) * f_xuefen;
					if (f_chengji < 60 && f_xuefen != 0)
					{
						char temp[512] = "<b style=\"color:#f6383a\">";
						strcat(temp, m_chengji);
						strcat(temp, "</b>");
						strncpy(m_chengji, temp, sizeof(m_chengji) - 1);
					}
					flags++;
					break;
				}
				case weiguoyuanyin:
				{
					mid(m_weiguoyuanyin, p1 + 18, p2 - p1 - 19, 0);
					//Trim(m_weiguoyuanyin);
					std::string  m_StrTmp = strformat(SCORE_TEMPLATE_BY_PLAN,
														(f_chengji < 60 && f_xuefen != 0) ? "background-color:rgba(255,0,0,0.5);color:#fff" : "",
														m_kechengming, m_shuxing, m_chengji, m_xuefen, f_jidian, m_weiguoyuanyin);
					unsigned int u8len = m_StrTmp.length() * 3 + 1;
					char *u8strtmp = (char *)malloc(u8len);
					gbk_to_utf8(m_StrTmp.c_str(), (unsigned int)m_StrTmp.length(), &u8strtmp, &u8len);

					m_Output.append(u8strtmp);
					free(u8strtmp);
					flags = kechenghao;
					memset(m_kechenghao, 0, 128);
					memset(m_kexuhao, 0, 128);
					memset(m_kechengming, 0, 1024);
					memset(m_yingwenkechengming, 0, 1024);
					memset(m_xuefen, 0, 64);
					memset(m_shuxing, 0, 64);
					memset(m_chengji, 0, 64);
					memset(m_weiguoyuanyin, 0, 2048);
					f_xuefen = 0.0;
					f_chengji = 0.0;
					f_jidian = 0.0;
					break;
				}
			}
			if (flags == weiguoyuanyin || flags == chengji)
			{
				p1 = strstr(p2 + 6, "<p align=\"center\">");
			}
			else
			{
				p1 = strstr(p2 + 6, "<td align=\"center\">");
			}
			
		}
		// 假如发生了错误
		if (!isSuccess)
		{
			Error(u8"<p>现在还没有出成绩或发生了错误噢</p>");
			return;
		}
		if (hasChengji == false)
		{
			m_Output.append(strformat(SCORE_TEMPLATE_BY_PLAN, "", u8"还没有任何成绩", "", "", "", 0.0, ""));
		}

		m_Output.append(AFTER_TEMPLATE);

		cout << GLOBAL_HEADER;

		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			std::string title(m_Student);
			title += u8"的专业方案 - ";
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat(m_lpszQuery.c_str(), m_Student, m_Output.c_str());
		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			cout << footer.c_str();
		}
		SetQueryCounter(++g_QueryCounter);
		return;
	}

	if (_GET(std::string(CGI_QUERY_STRING), "order") == "failed")
	{
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_FAILED, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>网络通信异常</p>");
			return;
		}
		
		unsigned int rep_len = req.GetLength() * 3 + 1;
		char *m_rep_body = (char *)malloc(rep_len);
		gbk_to_utf8(req.GetResult(), (unsigned int)req.GetLength(), &m_rep_body, &rep_len);
		req.GetResultString() = m_rep_body;
		free(m_rep_body);
		m_rep_body = req.GetResult();

		char *m_result = strstr(m_rep_body, "<table width=\"100%\"  border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\">");
		if (m_result == NULL)
		{
			Error(u8"<p><b>从服务器拉取分数失败 (BeginOfRet)</b></p><p>系统可能正忙，请稍后再试</p>");
			return;
		}
		m_result = strstr(m_result + 92, "<table width=\"100%\"  border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\">");
		if (m_result == NULL)
		{
			Error(u8"<p><b>从服务器拉取分数失败 (MidOfRet-Table)</b></p><p>系统可能正忙，请稍后再试</p>");
			return;
		}
		char *m_prep = (char *)malloc(req.GetLength());
		replace_string(m_result, "&nbsp;", "");
		replace_string(m_result, "\t", "");
		replace_string(m_result, "\r", "");
		replace_string(m_result, "\n", "");
		memset(m_prep, 0, req.GetLength());
		strncpy(m_prep, "<div id=\"list_page\">", req.GetLength() - 1);
		char *m_end_body = strstr(m_result, "</body>");
		if (m_result == NULL)
		{
			free(m_prep);
			Error(u8"<p>从服务器拉取分数失败 (EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			std::string title(m_Student);
			title += u8"的未通过科目 - ";
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			cout << footer.c_str();
		}

		SetQueryCounter(++g_QueryCounter);

		free(m_prep);
		return;
	}

	if (_GET(std::string(CGI_QUERY_STRING), "order") == "schedule")
	{
		CCurlTask req;
		if (!req.Exec(false, REQUEST_SCHEDULE, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>网络通信异常</p>");
			return;
		}
		
		unsigned int rep_len = req.GetLength() * 3 + 1;
		char *m_rep_body = (char *)malloc(rep_len);
		gbk_to_utf8(req.GetResult(), (unsigned int)req.GetLength(), &m_rep_body, &rep_len);
		req.GetResultString() = m_rep_body;
		free(m_rep_body);
		m_rep_body = req.GetResult();

		char *m_result = strstr(m_rep_body, "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"titleTop2\">");
		if (m_result == NULL)
		{
			Error(u8"<p><b>从服务器拉取课程表失败。(BeginOfTable)</b></p><p>建议你稍后再试</p>");
			return;
		}
		m_result += 81;
		char *m_prep = (char *)malloc(req.GetLength());
		replace_string(m_result, "&nbsp;", "");
		replace_string(m_result, "\t", "");
		replace_string(m_result, "\r", "");
		replace_string(m_result, "\n", "");

		// 因为上面已经做了u8转换，所以下面不再需要手工gbk编码了，直接U8替换。
		replace_string(m_result, /*"\xd0\xc7\xc6\xda\xd2\xbb", "\xd2\xbb"*/ u8"星期一", u8"一");
		replace_string(m_result, /*"\xd0\xc7\xc6\xda\xb6\xfe", "\xb6\xfe"*/ u8"星期二", u8"二");
		replace_string(m_result, /*"\xd0\xc7\xc6\xda\xc8\xfd", "\xc8\xfd"*/ u8"星期三", u8"三");
		replace_string(m_result, /*"\xd0\xc7\xc6\xda\xcb\xc4", "\xcb\xc4"*/ u8"星期四", u8"四");
		replace_string(m_result, /*"\xd0\xc7\xc6\xda\xce\xe5", "\xce\xe5"*/ u8"星期五", u8"五");
		replace_string(m_result, /*"\xd0\xc7\xc6\xda\xc1\xf9", "\xc1\xf9"*/ u8"星期六", u8"六");
		replace_string(m_result, /*"\xd0\xc7\xc6\xda\xc8\xd5", "\xc8\xd5"*/ u8"星期日", u8"日");

		memset(m_prep, 0, req.GetLength());
		strncpy(m_prep, "<div id=\"list_page\">", req.GetLength() - 1);
		char *m_end_body = strstr(m_result, "</table>");
		if (m_result == NULL)
		{
			free(m_prep);
			Error(u8"<p>从服务器拉取课程表失败。(EndOfBodyNotFound)</p>");
			return;
		}
		m_result -= 81;
		cout << GLOBAL_HEADER;
		std::string m_before = strformat("<a name=\"qb_731\"></a><table width=\"100%%\" border=\"0\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td class=\"Linetop\"></td></tr></tbody></table><table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\"><tbody><tr><td width=\"100%%\"><table border=\"0\" align=\"left\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td>&nbsp;</td><td valign=\"middle\">&nbsp;<b>%s</b> &nbsp;</td></tr></tbody></table></td></tr></tbody></table>", u8"我的课程表 / 选课结果");
		*(m_end_body + 26) = '<';
		*(m_end_body + 27) = '/';
		*(m_end_body + 28) = 'd';
		*(m_end_body + 29) = 'i';
		*(m_end_body + 30) = 'v';
		*(m_end_body + 31) = '>';
		*(m_end_body + 32) = '\0';

		strcat(m_prep, m_before.c_str());
		strcat(m_prep, m_result);

		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			std::string title(u8"本学期课程表 - ");
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat(m_lpszQuery.c_str(), m_Student, m_prep);
		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			cout << footer.c_str();
		}

		SetQueryCounter(++g_QueryCounter);
		free(m_prep);
		return;
	}

	// 定位到第一项成绩
	char *pStr1 = strstr((char *)req.GetResultString().c_str(),"<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
	bool hasChengji = true;
	if (pStr1 == NULL)
	{
		hasChengji = false;
	}

	bool m_success = false;
	std::string m_Output(BEFORE_TEMPLATE);
	char *pStr2 = NULL;
	char *pStr3 = NULL;

	// 循环获取每一项成绩信息

	double m_Total_xuefen = 0.0;
	double m_Total_pointsxxuefen = 0.0;
	double m_Total_jidian = 0.0;

	while (pStr1 != NULL) 
	{
		bool isPassed = true;
		pStr2 = pStr1;
		for (int i = 0; i < 3; i++)
		{
			if (pStr2 == NULL) break;
			pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		}
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subName[512] = { 0 };
		mid(m_subName, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subName);

		pStr2 = pStr3;
		pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subXuefen[128] = { 0 };
		mid(m_subXuefen, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subXuefen);

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_kcsx[128] = { 0 };
		mid(m_kcsx, pStr2, pStr3 - pStr2 - 19, 19);
		
		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuigaofen[128] = { 0 };
		mid(m_subzuigaofen, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subzuigaofen);

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuidifen[128] = { 0 };
		mid(m_subzuidifen, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subzuidifen);

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subjunfen[128] = { 0 };
		mid(m_subjunfen, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subjunfen);

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subchengji[256] = { 0 };
		mid(m_subchengji, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subchengji);

		if (strstr(m_subchengji, "\xd3\xc5\xd0\xe3" /*优秀*/) != NULL)
		{
			strncpy(m_subchengji,"95", sizeof(m_subchengji) - 1);
		}
		if (strstr(m_subchengji, "\xc1\xbc\xba\xc3" /*良好*/) != NULL)
		{
			strncpy(m_subchengji, "85", sizeof(m_subchengji) - 1);
		}
		if (strstr(m_subchengji, "\xd6\xd0\xb5\xc8" /*中等*/) != NULL)
		{
			strncpy(m_subchengji, "75", sizeof(m_subchengji) - 1);
		}
		if (strstr(m_subchengji, "\xbc\xb0\xb8\xf1" /*及格*/) != NULL)
		{
			if (atoi(m_subzuidifen) > 60)
			{
				strncpy(m_subchengji, m_subzuidifen, sizeof(m_subchengji) - 1);
			}
			else
			{
				strncpy(m_subchengji, "60", sizeof(m_subchengji) - 1);
			}
			
		}
		if (strstr(m_subchengji, "\xb2\xbb\xbc\xb0\xb8\xf1" /*不及格*/) != NULL)
		{
				strncpy(m_subchengji, "55", sizeof(m_subchengji) - 1);
				isPassed = false;
		}

		if (atof(m_subchengji) < 60) 
		{

			char m_completecj[256] = "<b style=\"color:#f6383a\">";
			strcat(m_completecj, m_subchengji);
			strcat(m_completecj, "</b>");
			memset(m_subchengji, 0, sizeof(m_subchengji));
			strncpy(m_subchengji, m_completecj, sizeof(m_subchengji) - 1);
			if (atof(m_subchengji) != 0 || atof(m_subzuidifen) != 0 || atof(m_subzuigaofen) != 0 || atof(m_subjunfen) != 0)
			{
				isPassed = false;
			}
		}

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_submingci[128] = { 0 };
		mid(m_submingci, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_submingci);

		// （分数x学分）全都加起来/总学分 = 加权分，排除体育和课程设计，任选类课程
		float m_xuefen = atof(m_subXuefen);
		float m_chengji = atof(m_subchengji);
		float m_kcxfjd = m_xuefen * cj2jd(m_chengji);
		if (strstr(m_kcsx, "\xc8\xce\xd1\xa1" /* 任选 */) == NULL && strstr(m_subName, "\xcc\xe5\xd3\xfd" /*体育*/) == NULL && strstr(m_subName, "\xbe\xfc\xca\xc2\xd1\xb5\xc1\xb7" /*军事训练*/) == NULL)
		{
			if (m_chengji != 0 || atof(m_subzuidifen) != 0 || atof(m_subzuigaofen) != 0 || atof(m_subjunfen) != 0)
			{
				m_Total_xuefen += m_xuefen;
			}
			double m_pointsxxuefen = m_xuefen * m_chengji;
			if (m_pointsxxuefen != 0)
			{
				m_Total_pointsxxuefen += m_pointsxxuefen;
			}
			m_Total_jidian += m_kcxfjd;
			/*if (m_chengji != 0)
			{
				double m_pointsxxuefen = m_xuefen * m_chengji;
				if (m_pointsxxuefen != 0)
				{
					m_Total_pointsxxuefen += m_pointsxxuefen;
				}
			}*/
		}

		
		std::string m_StrTmp = strformat(SCORE_TEMPLATE, isPassed ? "": "background-color:rgba(255,0,0,0.5);color:#fff", m_subName, m_kcsx, m_subchengji, m_subjunfen, m_subzuigaofen, m_subzuidifen,
			m_submingci, m_subXuefen, m_kcxfjd);
		
		unsigned int u8len = m_StrTmp.length() * 3 + 1;
		char *u8strtmp = (char *)malloc(u8len);
		gbk_to_utf8(m_StrTmp.c_str(), (unsigned int)m_StrTmp.length(), &u8strtmp, &u8len);

		m_Output.append(u8strtmp);

		free(u8strtmp);
		m_success = true; // 查到一个算一个
		pStr1 = strstr(pStr3, "<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
	}

	if (hasChengji == false)
	{
		m_Output.append(strformat(SCORE_TEMPLATE, "", u8"本学期还未出成绩", "", "", "", "", "", "", "", 0.0));
	}

	// 假如发生了错误
	if (!m_success) 
	{
		Error(u8"<p>现在还没有出成绩或发生了错误噢</p>");
		return;
	}

	m_Output.append(AFTER_TEMPLATE);

	// 填充返回页面
	float jiaquan, gpa;
	if (m_Total_pointsxxuefen != 0 || m_Total_xuefen != 0)
	{
		jiaquan = m_Total_pointsxxuefen / m_Total_xuefen;
		gpa = m_Total_jidian / m_Total_xuefen;
	}
	else
	{
		jiaquan = 0.0;
		gpa = 0.0;
	}
		std::string m_jiaquanfen = strformat(u8"<div id=\"i_total\"><p>加权平均分 / GPA (仅供参考)：</p><center>%.1f&nbsp;&nbsp;%.2f</center></div>", jiaquan, gpa);
		m_Output.insert(0, m_jiaquanfen);

	cout << GLOBAL_HEADER;

	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		std::string title(m_Student);
		title += u8"的本学期成绩 - ";
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	cout << strformat( m_lpszQuery.c_str(), m_Student, m_Output.c_str());
	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << footer.c_str();
	}

	SetQueryCounter(++g_QueryCounter);
}

// 处理 期中/月考/补考/缓考/清考等各种疑难杂考 (GET /query.fcgi?order=tests)
void parse_query_tests()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
		return;
	}

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	char m_Student[128] = { 0 };
	get_student_name(m_Student);

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0 || m_post_length > 4096)
	{
		Error(u8"<p>发生错误，POST 数据长度异常</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	std::string str_tests = _POST(post, "tests");
	if (str_tests.empty())
	{
		Error(u8"<p>发生错误，无法获取数据</p>");
		return;
	}
	char tests[4096] = { 0 };
	strncpy(tests, str_tests.c_str(), sizeof(tests) - 1);
	int len = url_decode(tests, strlen(tests));
	char temp[4096] = { 0 };
	left(temp, tests, len);
	memset(tests, 0, sizeof(tests));
	strncpy(tests, temp, sizeof(tests) - 1);

	if (len > 2048 || len <= 0)
	{
		Error(u8"<p>提交数据存在异常</p>");
		return;
	}

	char testName[4096] = { 0 };
	char termID[4096] = { 0 };
	char testID[4096] = { 0 };
	int matches = sscanf(tests, "%[^|]%*c%[^|]%*c%s", testName, termID, testID);
	if (matches != 3)
	{
		Error(u8"<p>提交数据存在异常</p>");
		return;
	}

	const char *postdata = "jxzxjh=%s&ksbh=%s&pageSize=300&page=1&currentPage=1&pageNo=";
	std::string request = strformat(postdata, termID, testID);

	CCurlTask req;
	if (!req.Exec(false, GET_TEST_DETAIL, CGI_HTTP_COOKIE, true, request))
	{
		Error(u8"<p><b>接收数据失败</b></p><p>网络通信异常</p>");
		return;
	}

	unsigned int rep_len = req.GetLength() * 3 + 1;
	char *m_rep_body = (char *)malloc(rep_len);
	gbk_to_utf8(req.GetResult(), (unsigned int)req.GetLength(), &m_rep_body, &rep_len);
	req.GetResultString() = m_rep_body;
	free(m_rep_body);
	m_rep_body = req.GetResult();

	char *m_result = strstr(m_rep_body, "<table cellpadding=\"0\" width=\"100%\" class=\"displayTag\" cellspacing=\"1\" border=\"0\" id=\"user\">");
	if (m_result == NULL)
	{
		Error(u8"<p><b>从服务器拉取分数失败 (BeginOfTable)</b></p><p>系统可能正忙，请稍后再试</p>");
		return;
	}

	m_result += 93;
	char *m_prep = (char *)malloc(req.GetLength());

	replace_string(m_result, "&nbsp;", "");
	replace_string(m_result, "\t", "");
	replace_string(m_result, "\r", "");
	replace_string(m_result, "\n", "");

	memset(m_prep, 0, req.GetLength());
	strncpy(m_prep, "<div id=\"list_page\">", req.GetLength() - 1);

	char *m_end_body = strstr(m_result, "</table>");
	if (m_result == NULL)
	{
		free(m_prep);
		Error(u8"<p>从服务器拉取分数失败 (EndOfBodyNotFound)</p>");
		return;
	}
	m_result -= 93;
	cout << GLOBAL_HEADER;
	std::string m_before = strformat("<a name=\"qb_731\"></a><table width=\"100%%\" border=\"0\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td class=\"Linetop\"></td></tr></tbody></table><table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\"><tbody><tr><td width=\"100%%\"><table border=\"0\" align=\"left\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td>&nbsp;</td><td valign=\"middle\">&nbsp;<b>%s</b> &nbsp;</td></tr></tbody></table></td></tr></tbody></table>", testName);
	*(m_end_body + 8) = '<';
	*(m_end_body + 9) = '/';
	*(m_end_body + 10) = 'd';
	*(m_end_body + 11) = 'i';
	*(m_end_body + 12) = 'v';
	*(m_end_body + 13) = '>';
	*(m_end_body + 14) = '\0';

	strcat(m_prep, m_before.c_str());
	strcat(m_prep, m_result);

	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		std::string title(m_Student);
		title += " - ";
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	cout << strformat(m_lpszQuery.c_str(), m_Student, m_prep);
	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << footer.c_str();
	}

	SetQueryCounter(++g_QueryCounter);

	free(m_prep);
	return;
}

// 获取学生姓名 (buffer 必须大于等于 36)
void get_student_name(char *p_lpszBuffer)
{
	memset(p_lpszBuffer, 0, 36);
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		strncpy(p_lpszBuffer, APP_NAME, 36 - 1);
		return;
	}

	CCurlTask req;
	if (!req.Exec(false, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE))
	{
		strncpy(p_lpszBuffer, APP_NAME, 36 - 1);
		return;
	}
	char *m_rep_header = req.GetResult();
	// 拉取学生姓名
	char *pStr1 = strstr(m_rep_header, "\x3c\x74\x64\x20\x63\x6c\x61\x73\x73\x3d\x22\x66\x69\x65\x6c\x64\x4e\x61\x6d\x65\x22\x3e\xd0\xd5\xc3\xfb\x3a\x26\x6e\x62\x73\x70\x3b\x3c\x2f\x74\x64\x3e" /*"<td class=\"fieldName\">姓名:&nbsp;</td>"*/);
	if (pStr1 == NULL)
	{
		strncpy(p_lpszBuffer, APP_NAME, 36 - 1);
		return;
	}
	pStr1 = strstr(pStr1 + 39, "<td>");
	if (pStr1 == NULL)
	{
		strncpy(p_lpszBuffer, APP_NAME, 36 - 1);
		return;
	}
	char *pStr2 = strstr(pStr1 + 5,"</td>");
	if (pStr2 == NULL)
	{
		strncpy(p_lpszBuffer, APP_NAME, 36 - 1);
		return;
	}
	mid(p_lpszBuffer, pStr1, pStr2 - pStr1 - 4, 4);
	if (strstr(p_lpszBuffer, "<td") != NULL || strstr(p_lpszBuffer, "td>") != NULL) //如果教务系统中没有姓名可以获取，则会获取到html乱码
	{
		strncpy(p_lpszBuffer, APP_NAME, 36 - 1);
		return;
	}
	unsigned int u8len = 512;
	char *temp = (char *)malloc(u8len);
	gbk_to_utf8(p_lpszBuffer, (unsigned int)strlen(p_lpszBuffer), &temp, &u8len);
	strncpy(p_lpszBuffer, temp, 36 - 1);
	free(temp);
}

// 获取学生帐号
void get_student_id(char *p_lpszBuffer)
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		*p_lpszBuffer = '\0';
		return;
	}

	CCurlTask req;
	if (!req.Exec(false, REQUEST_TOP, CGI_HTTP_COOKIE))
	{
		*p_lpszBuffer = '\0';
		return;
	}

	char *m_rep_header = req.GetResult();
	// 拉取学生姓名
	char *pStr1 = strstr(m_rep_header, "\xb5\xb1\xc7\xb0\xd3\xc3\xbb\xa7\x3a" /* 当前用户: */);
	if (pStr1 == NULL)
	{
		*p_lpszBuffer = '\0';
		return;
	}
	char *pStr2 = strstr(pStr1 + 8, "(");
	if (pStr2 == NULL)
	{
		*p_lpszBuffer = '\0';
		return;
	}
	mid(p_lpszBuffer, pStr1, pStr2 - pStr1 - 9, 9);
}

// 教务系统电子注册
int system_registration()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
		return -1;
	}

	// 发送请求，获取电子注册信息。
	CCurlTask req;
	if (!req.Exec(false, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>投递电子注册信息失败</b></p><p>网络通信异常</p>");
		return -1;
	}
	char *m_rep_header = req.GetResult();
	// 查找需要电子注册的学期信息。
	char *pStr1 = strstr(m_rep_header, "selected>");
	if (pStr1 == NULL)
	{
		Error(u8"<p><b>电子注册失败</b></p><p>本学期学费是否已交齐？请登录教务系统查看具体原因 (1)</p>");
		return -1;
	}
	pStr1 -= 70;
	char *pStr2 = strstr(pStr1, "<option value=\"");
	if (pStr2 == NULL)
	{
		Error(u8"<p><b>电子注册失败</b></p><p>请登录教务系统查看具体原因 (2)</p>");
		return -1;
	}
	pStr1 = pStr2;
	pStr2 = strstr(pStr1 + 16, "\"");
	if (pStr2 == NULL)
	{
		Error(u8"<p><b>电子注册失败</b></p><p>请登录教务系统查看具体原因 (3)</p>");
		return -1;
	}

	char m_regval[4096] = { 0 };
	mid(m_regval, pStr1, pStr2 - pStr1 - 15, 15);

	// 填充电子注册信息
	char m_post_reg_info[4096] = "zxjxjhh=";
	strcat(m_post_reg_info, m_regval);

	// 填充注册请求
	std::string m_post_req = strformat(REQUEST_POST_REGISTER_INTERFACE, m_regval);

	CCurlTask req2;
	// 开始电子注册
	if (!req2.Exec(false, m_post_req, CGI_HTTP_COOKIE, true, m_post_reg_info))
	{
		Error(u8"<p><b>电子注册失败</b></p><p>网络通信异常</p>");
		return -1;
	}
	m_rep_header = req2.GetResult();
	// 检查是否注册成功。
	pStr1 = strstr(m_rep_header, "\xd7\xa2\xb2\xe1\xb3\xc9\xb9\xa6" /*注册成功*/);
	if (pStr1 == NULL)
	{
		Error(u8"<p><b>电子注册失败</b></p><p>请登录教务系统查看具体原因 (4)</p>");
		return -1;
	}

	return 0;
}

// 登录学生
bool student_login(char *p_xuehao, char *p_password, char *p_captcha)
{
	// 发送登录请求。
	const char *m_origin = "zjh1=&tips=&lx=&evalue=&eflag=&fs=&dzslh=&zjh=%s&mm=%s&v_yzm=%s";
	std::string m_padding = strformat(m_origin, p_xuehao, p_password, p_captcha);

	CCurlTask req;
	if (!req.Exec(false, REQUEST_LOGIN, CGI_HTTP_COOKIE, true, m_padding))
	{
		Error(u8"<p><b>登录失败</b></p><p>网络通信异常</p>");
		return false;
	}
	char *m_rep_body = req.GetResult();

	// 拉取登录结果。
	char *m_result = m_rep_body;

	// 处理登录结果。
	char *m_login_not_auth = strstr(m_result, "\xd6\xa4\xbc\xfe\xba\xc5" /*"证件号"*/); // for some urp systems
	if (m_login_not_auth != NULL)
	{
		Error(u8"<p><b>学号或密码不对</b></p><p>如果修改过帐号密码，用新密码试一试</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "\xc3\xdc\xc2\xeb\xb2\xbb\xd5\xfd\xc8\xb7" /*密码不正确*/);
	if (m_login_not_auth != NULL)
	{
		Error(u8"<p><b>学号或密码不对</b></p><p>如果修改过帐号密码，用新密码试一试</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "\xd1\xe9\xd6\xa4\xc2\xeb\xb4\xed\xce\xf3" /*验证码错误*/);
	if (m_login_not_auth != NULL)
	{
		Error(u8"<p>验证码不对，请重试</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "\xca\xfd\xbe\xdd\xbf\xe2" /*数据库*/);
	if (m_login_not_auth != NULL)
	{
		Error(u8"<p>教务系统数据库繁忙</p><p>请稍后再试</p>");
		return false;
	}
	char *m_login_success = strstr(m_result, "\xd1\xa7\xb7\xd6\xd6\xc6\xd7\xdb\xba\xcf\xbd\xcc\xce\xf1" /*学分制综合教务*/);
	if (m_login_success == NULL)
	{
		Error(u8"<p>发生未知错误</p><p>请稍后再试</p>");
		return false;
	}

	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[1];
	MYSQL_BIND ret[1];
	memset(bind, 0, sizeof(bind));
	memset(ret, 0, sizeof(ret));
	std::string query("SELECT `id` FROM `userinfo` WHERE `id`=?");

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)p_xuehao;
	bind[0].buffer_length = strlen(p_xuehao);

	char id[36] = { 0 };
	ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	ret[0].buffer = (void *)id;
	ret[0].buffer_length = sizeof(id);

	if (stmt != NULL)
	{
		mysql_stmt_prepare(stmt, query.c_str(), query.length());
		mysql_stmt_bind_param(stmt, bind);
		mysql_stmt_bind_result(stmt, ret);
		mysql_stmt_execute(stmt);
		mysql_stmt_store_result(stmt);
		while (mysql_stmt_fetch(stmt) == 0);
		mysql_stmt_close(stmt);
	}

	// 对密码做URL解码
	size_t pass_len = strlen(p_password) + 1;
	int len = url_decode(p_password, pass_len - 1);
	char temp[4096] = { 0 };
	left(temp, p_password, len);
	memset(p_password, 0, pass_len);
	strncpy(p_password, temp, pass_len - 1);
	EnCodeStr(temp, temp); // 加密存放在temp，准备记入数据库

	if (strcmp(id, p_xuehao) != 0) // 无记录，则写入数据库
	{
		MYSQL_STMT *stmt = mysql_stmt_init(&db);
		MYSQL_BIND bind[3];
		memset(bind, 0, sizeof(bind));
		std::string query("INSERT INTO `userinfo` (`id`, `password`, `name`, `lastlogin`) VALUES (?, ?, ?, now())");

		if (stmt == NULL)
		{
			student_logout();
			std::string Err_Msg(u8"<b>很抱歉，登录失败</b><p>数据库错误 (statement 初始化失败)</p>");
			Error(Err_Msg.c_str());
			return false;
		}

		bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[0].buffer = (void *)p_xuehao;
		bind[0].buffer_length = strlen(p_xuehao);

		bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[1].buffer = (void *)temp;
		bind[1].buffer_length = strlen(temp);

		char m_stname[128] = { 0 };
		get_student_name(m_stname);
		bind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[2].buffer = (void *)m_stname;
		bind[2].buffer_length = strlen(m_stname);

		if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
			mysql_stmt_bind_param(stmt, bind) != 0 || 
			mysql_stmt_execute(stmt) != 0
			)
		{
			student_logout();
			std::string Err_Msg(u8"<b>很抱歉，登录失败</b><p>数据库错误 (");
			Err_Msg += mysql_stmt_error(stmt);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			mysql_stmt_close(stmt);
			return false;
		}
		mysql_stmt_close(stmt);
	}
	else // 为成功登录的学生更新记录
	{
		MYSQL_STMT *stmt = mysql_stmt_init(&db);
		MYSQL_BIND bind[3];
		memset(bind, 0, sizeof(bind));
		std::string query("UPDATE `userinfo` SET `password`=?, `name`=?, `lastlogin`=now() WHERE `id`=?");

		if (stmt == NULL)
		{
			student_logout();
			std::string Err_Msg(u8"<b>很抱歉，登录失败</b><p>数据库错误 (statement 初始化失败)</p>");
			Error(Err_Msg.c_str());
			return false;
		}

		bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[0].buffer = (void *)temp;
		bind[0].buffer_length = strlen(temp);

		char m_stname[128] = { 0 };
		get_student_name(m_stname);
		bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[1].buffer = (void *)m_stname;
		bind[1].buffer_length = strlen(m_stname);

		bind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[2].buffer = (void *)p_xuehao;
		bind[2].buffer_length = strlen(p_xuehao);

		if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
			mysql_stmt_bind_param(stmt, bind) != 0 || 
			mysql_stmt_execute(stmt) != 0)
		{
			student_logout();
			std::string Err_Msg(u8"<b>很抱歉，登录失败</b><p>数据库错误 (");
			Err_Msg += mysql_stmt_error(stmt);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			mysql_stmt_close(stmt);
			return false;
		}
		mysql_stmt_close(stmt);
	}
	// 至此，学生登录成功
	return true;
}

// 登出学生
void student_logout()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
		return;
	CCurlTask req;
	req.Exec(true, REQUEST_LOGOUT, CGI_HTTP_COOKIE);
}

// 学号快查入口 (/QuickQuery.fcgi)
void parse_QuickQuery_Intro()
{
	if (!ENABLE_QUICK_QUERY)
	{
		Error(u8"学号快速查询入口已关闭");
		return;
	}

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	cout << GLOBAL_HEADER;

	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		std::string title(u8"免密成绩查询 - ");
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	cout << strformat( m_lpszQuery.c_str(), APP_NAME);
	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << footer.c_str();
	}
}

// 学号快速查询结果 (/query.fcgi?act=QuickQuery)
void parse_QuickQuery_Result()
{
	bool m_need_update_cookie = false;
	std::string nullphoto;
	process_cookie(&m_need_update_cookie, nullphoto);

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0 || m_post_length >= 512)
	{
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
		Error(u8"<p>发生错误，POST 数据长度异常</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	// 获取学号
	std::string str_xuehao = _POST(post, "xh");
	if (str_xuehao.empty())
	{
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
		Error(u8"<p>无法获取学号信息</p>");
		return;
	}
	char m_xuehao[1024] = { 0 };
	strncpy(m_xuehao, str_xuehao.c_str(), sizeof(m_xuehao) - 1);
	if(strstr(m_xuehao, "%0D%0A") != NULL)
		replace_string(m_xuehao, "%0D%0A", "|");
	char *p = strtok(m_xuehao, "|");
	char *m_xh[512] = { NULL };
	int m_xhgs = 0;
	while (p)
	{
		m_xh[m_xhgs++] = p;
		p = strtok(NULL, "|");
	}
	if (m_xhgs > 5 || m_xhgs <= 0)
	{
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
		Error(u8"<p>输入的学号个数存在问题，请确认！</p>");
		return;
	}
	
	std::string m_list;
	char m_xxmz[512] = { 0 };

	std::string COOKIE("JSESSIONID=");
	if (m_need_update_cookie)
	{
		COOKIE.append(JSESSIONID);
	}
	else
	{
		COOKIE = CGI_HTTP_COOKIE;
	}

	for (int xh_index = 0; xh_index < m_xhgs; xh_index++)
		{
			if (strlen(m_xh[xh_index]) > 36)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>输入的学号中有长度存在问题，请确认！</p>");
				return;
			}

			std::string m_query_param = strformat("LS_XH=%s", m_xh[xh_index]);
			m_query_param.append("&resultPage=%3F"); // this is ok.

			CCurlTask req;
			if (!req.Exec(true, REQUEST_SET_REPORT_PARAMS, COOKIE.c_str(), true, m_query_param))
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>投递免密查询请求失败</p><p>请确认教务系统是可用的</p>");
				return;
			}

			char *m_lpvBuffer = req.GetResult();
			char *pStr1 = strstr(m_lpvBuffer, "&reportParamsId=");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>获取报表ID错误 (1)</p>");
				return;
			}
			char *pStr2 = strstr(pStr1 + 16, "\r\n");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>获取报表ID错误 (2)</p>");
				return;
			}
			char m_paramsID[512] = { 0 };
			mid(m_paramsID, pStr1 + 16, pStr2 - pStr1 - 16, 0);
			pStr1 = NULL;
			pStr2 = NULL;

			std::string m_query_report = strformat(REQUEST_REPORT_FILES, m_paramsID);
			CCurlTask req2;
			std::string referer(REFERER_REQUEST_REPORT_FILES);
			referer.insert(0, SERVER_URL);
			req2.SetReferer(referer);

			if (!req2.Exec(false, m_query_report, COOKIE.c_str()))
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>通过ID免密查询失败</p><p>发生了未知错误</p>");
				return;
			}
			m_lpvBuffer = req2.GetResult();
			pStr1 = strstr(m_lpvBuffer, "Exception: ");
			if (pStr1 != NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>教务系统出错了，免密查询失败，请稍后重试</p>");
				return;
			}
			pStr1 = NULL;
			pStr2 = NULL;

			pStr1 = strstr(m_lpvBuffer, "com.runqian.report.view.text.TextFileServlet");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>免密查询返回参数错误 (1)</p>");
				return;
			}
			pStr2 = strstr(pStr1 + 46, "\";");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>免密查询返回参数错误 (2)</p>");
				return;
			}
			char m_txt_req_path[512] = { 0 };
			mid(m_txt_req_path, pStr1 + 45, pStr2 - pStr1 - 45, 0);

			std::string m_query_score = strformat(REQUEST_TXT_SCORES, m_txt_req_path);
			CCurlTask req3;
			referer.erase();
			referer = strformat(REFERER_REQUEST_TXT_SCORES, m_paramsID);
			referer.insert(0, SERVER_URL);
			req3.SetReferer(referer);

			if (!req3.Exec(false, m_query_score, COOKIE.c_str()))
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>接收分数信息失败！</p>");
				return;
			}

			m_lpvBuffer = req3.GetResult();
			pStr1 = strstr(m_lpvBuffer, "\xd0\xd5\xc3\xfb\x09" /*姓名\t*/);
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>学生姓名获取失败 (1)</p>");
				return;
			}
			pStr2 = strstr(pStr1 + 4, "\t\t");
			if (pStr2 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>学生姓名获取失败 (2)</p>");
				return;
			}

			if ((pStr2 - pStr1) <= 4)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(strformat(u8"<p><b>出现错误，请确认输入正确</b></p><p>发生错误的学号: %s</p>", m_xh[xh_index]).c_str());
				return;
			}
			mid(m_xxmz, pStr1 + 4, pStr2 - pStr1 - 5, 1);
			if (strlen(m_xxmz) < 2)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(strformat(u8"<p><b>获取信息失败，请确认输入正确</b></p><p>发生错误的学号: %s</p>", m_xh[xh_index]).c_str());
				return;
			}
			std::string m_xxmz_div = strformat("<div class=\"weui-cells__title\">%s</div>", m_xxmz);

			unsigned int u8len = m_xxmz_div.length() * 3 + 1;
			char *m_xxmz_htmlu8 = (char *)malloc(u8len);
			gbk_to_utf8(m_xxmz_div.c_str(), (unsigned int)m_xxmz_div.length(), &m_xxmz_htmlu8, &u8len);
			std::string m_xxmz_html(m_xxmz_htmlu8);
			free(m_xxmz_htmlu8);

			pStr1 = NULL;
			pStr2 = NULL;
			pStr1 = strstr(m_lpvBuffer, "\xbf\xbc\xca\xd4\xca\xb1\xbc\xe4\x09\x09\x0a" /*"考试时间\t\t\n"*/);
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(strformat(u8"<p><b>接收到的数据存在问题</b></p><p>发生错误的学号: %s</p>", m_xh[xh_index]).c_str());
				return;
			}

			pStr1 += 11;
			pStr2 = strstr(pStr1, "\t\t\t\t");
			bool m_success = true;
			m_list.append(m_xxmz_html);
			m_list.append("<div class=\"weui-cells\">");
			test_info m_test_info[256];
			int m_index = 0;

			while (pStr2 != NULL)
			{
				char m_kcmz[128] = { 0 };
				mid(m_kcmz, pStr1, pStr2 - pStr1, 0);
				//cout << m_kcmz;

				pStr1 = pStr2 + 4;
				pStr2 = strstr(pStr1, "\t");
				if (pStr2 == NULL)
				{
					m_success = false;
					break;
				}

				char m_xf[64] = { 0 };
				mid(m_xf, pStr1, pStr2 - pStr1, 0);
				pStr1 = pStr2 + 1; // 学分
				pStr2 = strstr(pStr1, "\t");
				if (pStr2 == NULL)
				{
					m_success = false;
					break;
				}

				char m_cj[64] = { 0 };
				mid(m_cj, pStr1, pStr2 - pStr1, 0);
				if (atoi(m_cj) < 60)
				{
					char m_red[64] = "<b style=\"color:#f6383a\">";
					strcat(m_red, m_cj);
					strcat(m_red, "</b>");
					memset(m_cj, 0, sizeof(m_red));
					strncpy(m_cj, m_red, sizeof(m_red) - 1);
				}

				//cout << " - " << m_cj;
				pStr2 = strstr(pStr2 + 1, "\t");
				if (pStr2 == NULL)
				{
					m_success = false;
					break;
				}

				char m_lb[64] = { 0 };
				mid(m_lb, pStr1, pStr2 - pStr1, 0);
				if (strstr(m_lb, "\xd6\xd8\xd0\xde" /*"重修"*/) != NULL)
				{
					char m_kcmz_cx[256] = { 0 };
					strcat(m_kcmz_cx, "\x3c\x62\x20\x73\x74\x79\x6c\x65\x3d\x22\x63\x6f\x6c\x6f\x72\x3a\x23\x66\x35\x37\x63\x30\x30\x22\x3e\x5b\xd6\xd8\xd0\xde\x5d\x3c\x2f\x62\x3e\x20" /*"<b style=\"color:#f57c00\">[重修]</b> "*/); // 这里不用做U8转换，因为下面做过了
					strcat(m_kcmz_cx, m_kcmz);
					memset(m_kcmz, 0, sizeof(m_kcmz));
					strncpy(m_kcmz, m_kcmz_cx, sizeof(m_kcmz) - 1);
				}

				//cout << " - " << m_lb;
				pStr1 = pStr2 + 6;
				pStr2 = strstr(pStr1, "\t\t");
				if (pStr2 == NULL)
				{
					m_success = false;
					break;
				}

				char m_date[128] = { 0 };
				mid(m_date, pStr1, pStr2 - pStr1, 0);
				char m_date_4[64] = { 0 };
				left(m_date_4, m_date, 6);


				//cout << " - " << m_date << endl;
				pStr1 = pStr2 + 2;
				pStr2 = strstr(pStr1, "\t\t\t\t");

				if (atoi(m_xf) == 0 && atoi(m_cj) == 0)
				{
					break;
				}

				strncpy(m_test_info[m_index].cj, m_cj, sizeof(m_test_info[m_index].cj) - 1);
				strncpy(m_test_info[m_index].kcmz, m_kcmz, sizeof(m_test_info[m_index].kcmz) - 1);
				m_test_info[m_index].date = atoi(m_date_4);
				m_index++;
			}

			if (!m_success)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
				Error(u8"<p>还没有出成绩或者发生了错误</p>");
				return;
			}

			int m_max_date = 0;
			int m_secondary_max = 0;

			for (int i = 0; i < m_index; i++)
			{
				if (m_test_info[i].date > m_max_date)
				{
					m_max_date = m_test_info[i].date;
				}
			}

			for (int i = 0; i < m_index; i++)
			{
				if (m_test_info[i].date != m_max_date && m_test_info[i].date > m_secondary_max)
				{
					m_secondary_max = m_test_info[i].date;
				}
			}

			int m_interval = m_max_date - m_secondary_max;

			for (int i = 0; i < m_index; i++)
			{
				if (m_interval >= 3) // 如果两次成绩相隔大于3个月，则显示当期成绩
				{
					if (m_test_info[i].date == m_max_date)
					{
						std::string m_temp = strformat(QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
						unsigned int u8len = 4096;
						char *m_u8tmp = (char *)malloc(u8len);
						gbk_to_utf8(m_temp.c_str(), (unsigned int)m_temp.length(), &m_u8tmp, &u8len);
						m_list.append(m_u8tmp);
						free(m_u8tmp);
					}
				}
				else if (m_test_info[i].date == m_max_date || m_test_info[i].date == m_secondary_max)
				{
					std::string m_temp = strformat(QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
					unsigned int u8len = 4096;
					char *m_u8tmp = (char *)malloc(u8len);
					gbk_to_utf8(m_temp.c_str(), (unsigned int)m_temp.length(), &m_u8tmp, &u8len);
					m_list.append(m_u8tmp);
					free(m_u8tmp);
				}
			}
			m_list.append("</div>");
		}

		cout << GLOBAL_HEADER;

		if (m_xhgs > 1)
		{
			if (!CGI_HTTP_X_IS_AJAX_REQUEST)
			{
				std::string title(u8"多人查询 - 免密成绩查询 - ");
				title += APP_NAME;
				cout << strformat(header.c_str(), title.c_str());
			}
			cout << strformat( m_lpszQuery.c_str(), u8"多人查询", m_list.c_str());
		}
		else
		{
			unsigned int u8len = 512;
			char *m_xxmz_u8 = (char *)malloc(u8len);
			gbk_to_utf8(m_xxmz, (unsigned int)strlen(m_xxmz), &m_xxmz_u8, &u8len);

			if (!CGI_HTTP_X_IS_AJAX_REQUEST)
			{
				std::string title(m_xxmz_u8);
				title += u8" - 免密成绩查询 - ";
				title += APP_NAME;
				cout << strformat(header.c_str(), title.c_str());
			}
			cout << strformat( m_lpszQuery.c_str(), m_xxmz_u8, m_list.c_str());
			free(m_xxmz_u8);
		}
		if (!CGI_HTTP_X_IS_AJAX_REQUEST)
		{
			cout << footer.c_str();
		}

		g_QueryCounter += m_xhgs;
		SetQueryCounter(g_QueryCounter);
}

// 微信帐号绑定入口与解绑逻辑 (/OAuth2.fcgi?act=link)
void OAuth2_linking(bool isPOST)
{
	// 解除绑定逻辑
	std::string str_release = _GET(std::string(CGI_QUERY_STRING), "release");
	if (!str_release.empty())
	{
		char student_id[512] = { 0 };
		get_student_id(student_id);
		if (strlen(student_id) == 0)
		{
			Error(u8"非法操作 (尚未登录)");
			return;
		}

		size_t qslen = strlen(CGI_QUERY_STRING);
		char *releaseid = new char[qslen];
		memset(releaseid, 0, qslen);
		strncpy(releaseid, str_release.c_str(), qslen - 1);

		if (strcmp(releaseid, student_id) != 0)
		{
			Error(u8"非法操作 (身份错误)");
			return;
		}

		MYSQL_STMT *stmt = mysql_stmt_init(&db);
		MYSQL_BIND bind[1];
		memset(bind, 0, sizeof(bind));
		std::string query("UPDATE `userinfo` SET `openid`=NULL, `OAuth_name`=NULL, `OAuth_avatar`=NULL WHERE `id`=?");

		if (stmt == NULL)
		{
			Error(u8"<b>解除绑定失败</b><p>数据库错误 (statement 初始化失败)</p>");
			return;
		}

		bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[0].buffer = (void *)student_id;
		bind[0].buffer_length = strlen(student_id);

		if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
			mysql_stmt_bind_param(stmt, bind) != 0 || 
			mysql_stmt_execute(stmt) != 0)
		{
			std::string Err_Msg(u8"<b>解除绑定失败</b><p>数据库错误 (");
			Err_Msg += mysql_stmt_error(stmt);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			mysql_stmt_close(stmt);
			return;
		}

		mysql_stmt_close(stmt);
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/main.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}
	std::string str_openid = _GET(std::string(CGI_QUERY_STRING), "openid");
	if (str_openid.empty())
	{
		Error(u8"鉴权失败 (openid)");
		return;
	}

	std::string access_token = _GET(std::string(CGI_QUERY_STRING), "proc");
	if (access_token.empty())
	{
		Error(u8"没有过程状态信息 (proc)");
		return;
	}

	if (!isPOST)
	{
		// 如果传进 user，则自动填写学号、并且从数据库中拿密码。
		std::string str_stid = _GET(std::string(CGI_QUERY_STRING), "user");
		char stid[128] = { 0 };
		if (!str_stid.empty())
		{
			strncpy(stid, str_stid.c_str(), sizeof(stid) - 1);
			DeCodeStr(stid);
		}

		char pass[1024] = {0};
		if (strlen(stid) != 0 && strcmp(stid, "NONE") != 0)
		{
			MYSQL_STMT *stmt = mysql_stmt_init(&db);
			MYSQL_BIND bind[1];
			MYSQL_BIND ret[1];
			memset(bind, 0, sizeof(bind));
			memset(ret, 0, sizeof(ret));
			std::string query("SELECT `password` FROM `userinfo` WHERE `id`=?");

			bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
			bind[0].buffer = (void *)stid;
			bind[0].buffer_length = strlen(stid);

			ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
			ret[0].buffer = (void *)pass;
			ret[0].buffer_length = sizeof(pass);

			if (stmt != NULL)
			{
				mysql_stmt_prepare(stmt, query.c_str(), query.length());
				mysql_stmt_bind_param(stmt, bind);
				mysql_stmt_bind_result(stmt, ret);
				mysql_stmt_execute(stmt);
				mysql_stmt_store_result(stmt);
				while (mysql_stmt_fetch(stmt) == 0);
				mysql_stmt_close(stmt);
				DeCodeStr(pass);
			}
		}

		std::string OAUTH2_HTML = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

		cout << GLOBAL_HEADER;

		std::string title(u8"微信用户绑定 - ");
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());

		char access_token_dec[1024] = { 0 };
		strncpy(access_token_dec, access_token.c_str(), sizeof(access_token_dec) - 1);
		DeCodeStr(access_token_dec);
		std::string m_OAuth_name;
		std::string m_OAuth_avatar;
		PullOAuthUserInfo(access_token_dec, (char *)str_openid.c_str(), m_OAuth_name, m_OAuth_avatar);

		if (strlen(stid) == 0 || strcmp(stid, "NONE") == 0)
		{
			cout << strformat(OAUTH2_HTML.c_str(), APP_NAME, u8"微信用户绑定", str_openid.c_str(), access_token.c_str(),
								(!m_OAuth_name.empty() && !m_OAuth_avatar.empty()) ? strformat(LOGGED_USER_HTML, m_OAuth_avatar.c_str(), m_OAuth_name.c_str()).c_str() : "",
								 "", pass);
		}
		else if(strlen(pass) == 0)
		{
			cout << strformat(OAUTH2_HTML.c_str(), APP_NAME, u8"微信用户绑定", str_openid.c_str(), access_token.c_str(),
								(!m_OAuth_name.empty() && !m_OAuth_avatar.empty()) ? strformat(LOGGED_USER_HTML, m_OAuth_avatar.c_str(), m_OAuth_name.c_str()).c_str() : "",
								stid, "");
		}
		else
		{
			cout << strformat(OAUTH2_HTML.c_str(), APP_NAME, u8"微信用户绑定", str_openid.c_str(), access_token.c_str(),
								(!m_OAuth_name.empty() && !m_OAuth_avatar.empty()) ? strformat(LOGGED_USER_HTML, m_OAuth_avatar.c_str(), m_OAuth_name.c_str()).c_str() : "",
								stid, pass);
		}
		cout << footer.c_str();
	}
	else // 提交帐号密码验证码，打算登录绑定了
	{
		// 获取 POST 数据。
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0 || m_post_length > 127)
		{
			Error(u8"<p>输入的帐号或密码有问题，请重试</p>");
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
		std::string post(m_post_data);
		free(m_post_data);

		// 获取学号
		std::string str_xuehao = _POST(post, "xh");
		if (str_xuehao.empty())
		{
			Error(u8"<p>无法获取学号信息</p>");
			return;
		}
		char m_xuehao[128] = { 0 };
		strncpy(m_xuehao, str_xuehao.c_str(), sizeof(m_xuehao) - 1);

		// 获取密码
		std::string str_password = _POST(post, "mm");
		if (str_password.empty())
		{
			Error(u8"<p>无法获取密码信息</p>");
			return;
		}
		char m_password[MAX_PATH] = { 0 };
		strncpy(m_password, str_password.c_str(), sizeof(m_password) - 1);

		// 获取验证码
		std::string str_captcha = _POST(post, "yzm");
		if (str_captcha.empty())
		{
			Error(u8"<p>无法获取验证码信息</p>");
			return;
		}
		char m_captcha[128] = { 0 };
		strncpy(m_captcha, str_captcha.c_str(), sizeof(m_captcha) - 1);

		// 取得了学号密码验证码，下面先检查该账号是否已绑定微信帐号
		MYSQL_STMT *idcheck_stmt = mysql_stmt_init(&db);
		if (idcheck_stmt == NULL)
		{
			Error(u8"<b>很抱歉，微信绑定失败</b><p>数据库错误，请稍后再试</p>");
			return;
		}
		MYSQL_BIND idcheck_bind[1];
		MYSQL_BIND idcheck_query_ret[1];
		my_bool idcheck_is_null[1];
		memset(idcheck_query_ret, 0, sizeof(idcheck_query_ret));
		memset(idcheck_is_null, 0, sizeof(idcheck_is_null));
		std::string idcheck_query("SELECT `openid` FROM `userinfo` WHERE `id`=?");
		char *idcheck_openid = (char *)malloc(MAX_PATH);
		memset(idcheck_openid, 0, MAX_PATH);
		idcheck_bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		idcheck_bind[0].buffer = (void *)m_xuehao;
		idcheck_bind[0].buffer_length = strlen(m_xuehao);
		idcheck_query_ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		idcheck_query_ret[0].buffer = (void *)idcheck_openid;
		if (mysql_stmt_prepare(idcheck_stmt, idcheck_query.c_str(), idcheck_query.length()) != 0 ||
			mysql_stmt_bind_param(idcheck_stmt, idcheck_bind) != 0 ||
			mysql_stmt_bind_result(idcheck_stmt, idcheck_query_ret) != 0 ||
			mysql_stmt_store_result(idcheck_stmt) != 0 ||
			mysql_stmt_execute(idcheck_stmt) != 0
			)
		{
			free(idcheck_openid);
			char Err_Msg[1024] = u8"<b>很抱歉，微信绑定失败</b><p>数据库错误 (";
			strcat(Err_Msg, mysql_stmt_error(idcheck_stmt));
			strcat(Err_Msg, u8")</p><p>请稍后再试</p>");
			Error(Err_Msg);
			mysql_stmt_close(idcheck_stmt);
			return;
		}
		mysql_stmt_fetch(idcheck_stmt);
		if (!idcheck_is_null[0] || strlen(idcheck_openid) != 0) {
			free(idcheck_openid);
			char Err_Msg[1024] = u8"<b>学号已有绑定的微信帐号</b><p>如需更换，请先解除原有绑定</p>";
			Error(Err_Msg);
			mysql_stmt_close(idcheck_stmt);
			return;
		}
		free(idcheck_openid);
		mysql_stmt_close(idcheck_stmt);

		// 如果学号没有绑定过微信，那么可以放行绑定，继续操作，登录学生
		if (!student_login(m_xuehao, m_password, m_captcha))
		{
			return;
		}

		// 这里表示登录成功，应该写入数据库了。
		MYSQL_STMT *stmt = mysql_stmt_init(&db);
		MYSQL_BIND bind[2];
		memset(bind, 0, sizeof(bind));
		std::string query("UPDATE `userinfo` SET `openid`=? WHERE `id`=?");

		if (stmt == NULL)
		{
			Error(u8"<b>很抱歉，微信绑定失败</b><p>数据库错误，请稍后再试</p>");
			return;
		}

		bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[0].buffer = (void *)str_openid.c_str();
		bind[0].buffer_length = str_openid.length();

		bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[1].buffer = (void *)m_xuehao;
		bind[1].buffer_length = strlen(m_xuehao);

		if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
			mysql_stmt_bind_param(stmt, bind) != 0 || 
			mysql_stmt_execute(stmt) != 0
			)
		{
			char Err_Msg[1024] = u8"<b>很抱歉，微信绑定失败</b><p>数据库错误 (";
			strcat(Err_Msg, mysql_stmt_error(stmt));
			strcat(Err_Msg, u8")</p><p>请稍后再试</p>");
			Error(Err_Msg);
			mysql_stmt_close(stmt);
			return;
		}
		mysql_stmt_close(stmt);

		char *mem_access_token = (char *)malloc(MAX_PATH);
		memset(mem_access_token, 0, MAX_PATH);
		strncpy(mem_access_token, access_token.c_str(), MAX_PATH - 1);
		DeCodeStr(mem_access_token);
		WriteOAuthUserInfo(mem_access_token, (char *)str_openid.c_str(), m_xuehao);

		cout << "Status: 302 Found\r\n";
		output_token_header(m_xuehao, m_password);
		cout << "Location: " << getAppURL().c_str() << "/main.fcgi\r\n";
		cout << GLOBAL_HEADER;

		free(mem_access_token);
	}
	return;
}

// 教学评估页面 (/TeachEval.fcgi)
void parse_teaching_evaluation()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // 还没登录就丢去登录。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
		return;
	}

	// 检查是否需要教学评估
	CCurlTask req;
	if (!req.Exec(false, GET_TEACH_EVAL_LIST, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>教学评估请求投递失败</b></p><p>网络通信异常</p>");
		return;
	}

	char *m_rep_body = req.GetResult();
	char *m_result = strstr(m_rep_body, "\xd1\xa7\xc9\xfa\xc6\xc0\xb9\xc0\xce\xca\xbe\xed\xc1\xd0\xb1\xed" /*"学生评估问卷列表"*/);
	if (m_result == NULL)
	{
		Error(u8"<p>从服务器拉取教学评估信息失败</p>");
		return;
	}

	m_result = strstr(m_rep_body, "\xb7\xc7\xbd\xcc\xd1\xa7\xc6\xc0\xb9\xc0\xca\xb1\xc6\xda\xa3\xac\xbb\xf2\xc6\xc0\xb9\xc0\xca\xb1\xbc\xe4\xd2\xd1\xb9\xfd" /*非教学评估时期，或评估时间已过*/);
	if (m_result != NULL)
	{
		Error(u8"<p>学院还没有开放评教呢，或者你来晚了哦</p>");
		return;
	}
	m_result = strstr(m_rep_body, "\xc4\xfa\xc3\xbb\xd3\xd0\xbf\xc9\xbb\xd8\xb4\xf0\xb5\xc4\xce\xca\xbe\xed" /*您没有可回答的问卷*/);
	if (m_result != NULL)
	{
		Error(u8"<p>您没有可回答的问卷，无需教学评估哦</p>");
		return;
	}

	int counts = 0;
	teach_eval te[200];
	char *m_result1 = strstr(m_rep_body, "\x3C\x74\x64\x20\x61\x6C\x69\x67\x6E\x3D\x22\x63\x65\x6E\x74\x65\x72\x22\x3E\xCA\xC7\x3C\x2F\x74\x64\x3E" /*"<td align=\"center\">是</td>"*/);
	m_result = strstr(m_rep_body, "<img name=\"");

	while (m_result != NULL)
	{
		if (m_result1 != NULL)
		{
			te[counts].evaled = true;
		}
		char *m_result2 = strstr(m_result + 11, "\"");

		if (m_result2 == NULL)
		{
			Error(u8"<p>从服务器拉取待评列表失败</p>");
			return;
		}
		char img_txt[128] = { 0 };
		mid(img_txt, m_result + 11, m_result2 - m_result - 11, 0);
		char dst[10][128];
		memset(dst, 0, sizeof(dst));

		int split_ret = split(dst, img_txt, "#@");

		if (split_ret != 6)
		{
			Error(u8"<p>评教条目数目分割失败</p>");
			return;
		}

		strncpy(te[counts].wjbm, dst[0], sizeof(te[counts].wjbm) - 1);
		strncpy(te[counts].bpr, dst[1], sizeof(te[counts].bpr) - 1);
		strncpy(te[counts].pgnr, dst[5], sizeof(te[counts].pgnr) - 1);
		strncpy(te[counts].bprm, dst[2], sizeof(te[counts].bprm) - 1);
		unsigned int len = strlen(dst[4]);
		unsigned int to_len = len * 3 + 1;
		char *tmp = (char *)malloc(to_len);
		gbk_to_utf8(dst[4], len, &tmp, &to_len);
		strncpy(te[counts].name, tmp, sizeof(te[counts].name) - 1);
		free(tmp);

		counts++;
		m_result1 = strstr(m_result, "\x3C\x74\x64\x20\x61\x6C\x69\x67\x6E\x3D\x22\x63\x65\x6E\x74\x65\x72\x22\x3E\xCA\xC7\x3C\x2F\x74\x64\x3E" /*"<td align=\"center\">是</td>"*/);
		m_result = strstr(m_result + 11, "<img name=\"");
	}

	int to_eval = 0;
	std::string to_eval_list = "<div class=\"weui-cells\">";
	for (int i = 0; i < counts; i++)
	{
		to_eval_list += "<div class=\"weui-cell\"><div class=\"weui-cell__bd\"><p><span style=\"color:#4a82e5\">[";
			unsigned int len = strlen(te[i].bprm);
			unsigned int to_len = len * 3 + 1;
			char *tmp = (char *)malloc(to_len);
			gbk_to_utf8(te[i].bprm, len, &tmp, &to_len);
			to_eval_list += tmp;
			to_eval_list += "]&nbsp;</span>";
			free(tmp);
			to_eval_list += te[i].name;
			to_eval_list += "</p></div><div class=\"weui-cell__ft\">";
			if (te[i].evaled == false)
			{
				to_eval_list += u8"未评价";
				to_eval++;
			}
			else
			{
				to_eval_list += u8"<span style=\"color:#00a70e\">已评价</span>";
			}
			to_eval_list += "</div></div>";
	}
	to_eval_list += "</div>";

	std::string m_lpszTeachEvalPage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	std::string outer;
	std::string out_head;

	cout << GLOBAL_HEADER;

	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		std::string title = u8"一键评教 - ";
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}

	bool need_eval = true;
	if (to_eval && counts)
	{
		out_head = strformat(u8"<div class=\"weui-cells__title\">嗯，当前还有 %d 门课程需要评估，总共 %d 门</div>", to_eval, counts);
	}
	else
	{
		out_head = u8"<div class=\"weui-cells__title\"><p>嗯，你都评价好啦</div>";
		need_eval = false;
	}

	outer.append(out_head);
	outer.append(to_eval_list);

	cout << strformat(
		m_lpszTeachEvalPage.c_str(),
		need_eval ? u8"老师很辛苦，给个赞呗。一键全好评，你懂的 :)" : "",
		need_eval ? "block" : "none"
		, outer.c_str());
	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << footer.c_str();
	}
}

// 教学评估流程 (POST /TeachEval.fcgi?act=Evaluate)
void teaching_evaluation()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // 还没登录就丢去登录。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
		return;
	}

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0 || m_post_length > 10240)
	{
		Error(u8"<p>发生错误，POST 数据长度异常</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	// 获取主观评价
	std::string str_zgpj = _POST(post, "nr");
	if (str_zgpj.empty())
	{
		Error(u8"<p>无法获取主观评价内容</p>");
		return;
	}

	char zgpj[10240] = { 0 };
	strncpy(zgpj, str_zgpj.c_str(), sizeof(zgpj) - 1);
	int len = url_decode(zgpj, strlen(zgpj));
	char *temp = (char *)malloc(10240);
	memset(temp, 0, 10240);
	left(temp, zgpj, len);
	unsigned int dlen = 10240;
	char *decoded = (char *)malloc(dlen);
	utf8_to_gbk(temp, strlen(temp), &decoded, &dlen);
	free(temp);
	int nlen = 0;
	char *final_text = url_encode(decoded, strlen(decoded), &nlen);
	memset(zgpj, 0, sizeof(zgpj));
	strncpy(zgpj, final_text, sizeof(zgpj) - 1);
	free(decoded);
	free(final_text);

	// 检查是否需要教学评估
	CCurlTask req;
	if (!req.Exec(false, GET_TEACH_EVAL_LIST, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>教学评估请求投递失败</b></p><p>网络通信异常</p>");
		return;
	}

	char *m_rep_body = req.GetResult();
	char *m_result = strstr(m_rep_body, "\xd1\xa7\xc9\xfa\xc6\xc0\xb9\xc0\xce\xca\xbe\xed\xc1\xd0\xb1\xed" /*"学生评估问卷列表"*/);
	if (m_result == NULL)
	{
		Error(u8"<p>从服务器拉取教学评估信息失败</p>");
		return;
	}

	m_result = strstr(m_rep_body, "\xb7\xc7\xbd\xcc\xd1\xa7\xc6\xc0\xb9\xc0\xca\xb1\xc6\xda\xa3\xac\xbb\xf2\xc6\xc0\xb9\xc0\xca\xb1\xbc\xe4\xd2\xd1\xb9\xfd" /*非教学评估时期，或评估时间已过*/);
	if (m_result != NULL)
	{
		Error(u8"<p>学院还没有开放评教呢，或者你来晚了哦</p>");
		return;
	}
	m_result = strstr(m_rep_body, "\xc4\xfa\xc3\xbb\xd3\xd0\xbf\xc9\xbb\xd8\xb4\xf0\xb5\xc4\xce\xca\xbe\xed" /*您没有可回答的问卷*/);
	if (m_result != NULL)
	{
		Error(u8"<p>您没有可回答的问卷，无需教学评估哦</p>");
		return;
	}

	int counts = 0;
	teach_eval te[200];
	char *m_result1 = strstr(m_rep_body, "\x3C\x74\x64\x20\x61\x6C\x69\x67\x6E\x3D\x22\x63\x65\x6E\x74\x65\x72\x22\x3E\xCA\xC7\x3C\x2F\x74\x64\x3E" /*"<td align=\"center\">是</td>"*/);
	m_result = strstr(m_rep_body, "<img name=\"");

	while (m_result != NULL)
	{
		if (m_result1 != NULL)
		{
			te[counts].evaled = true;
		}
		char *m_result2 = strstr(m_result + 11, "\"");

		if (m_result2 == NULL)
		{
			Error(u8"<p>从服务器拉取待评列表失败</p>");
			return;
		}
		char img_txt[128] = { 0 };
		mid(img_txt, m_result + 11, m_result2 - m_result - 11, 0);
		char dst[10][128];
		memset(dst, 0, sizeof(dst));

		int split_ret = split(dst, img_txt, "#@");

		if (split_ret != 6)
		{
			Error(u8"<p>评教条目数目分割失败</p>");
			return;
		}

		strncpy(te[counts].wjbm, dst[0], sizeof(te[counts].wjbm) - 1);
		strncpy(te[counts].bpr, dst[1], sizeof(te[counts].bpr) - 1);
		strncpy(te[counts].pgnr, dst[5], sizeof(te[counts].pgnr) - 1);
		strncpy(te[counts].name, dst[4], sizeof(te[counts].name) - 1);
		int new_len = 0;
		char *tmp = url_encode(dst[3], strlen(dst[3]), &new_len);
		left(te[counts].wjmc, tmp, new_len);
		free(tmp);
		char *tmp2 = url_encode(dst[2], strlen(dst[2]), &new_len);
		left(te[counts].bprm, tmp2, new_len);
		free(tmp2);
		char *tmp3 = url_encode(dst[4], strlen(dst[4]), &new_len);
		left(te[counts].pgnrm, tmp3, new_len);
		free(tmp3);

		counts++;
		m_result1 = strstr(m_result, "\x3C\x74\x64\x20\x61\x6C\x69\x67\x6E\x3D\x22\x63\x65\x6E\x74\x65\x72\x22\x3E\xCA\xC7\x3C\x2F\x74\x64\x3E" /*"<td align=\"center\">是</td>"*/);
		m_result = strstr(m_result + 11, "<img name=\"");
	}

	int to_eval = 0;
	for (int i = 0; i < counts; i++)
	{
		if (te[i].evaled == false)
			to_eval++;
	}

	char last[64] = {0};
	if (to_eval && counts)
	{
		for (int i = 0; i < counts; i++)
		{
			if (te[i].evaled == false)
			{
				std::string pre_post = "wjbm=";
				pre_post = pre_post + te[i].wjbm + "&bpr=" + te[i].bpr + "&pgnr=" + te[i].pgnr + "&oper=wjShow&wjmc=" + te[i].wjmc + "&bprm=" + te[i].bprm + "&pgnrm=" + te[i].pgnrm + "&wjbz=null&pageSize=20&page=1&currentPage=1&pageNo=";

				CCurlTask req2;
				if (!req2.Exec(false, POST_PRE_TEACH_EVAL, CGI_HTTP_COOKIE, true, pre_post.c_str()))
				{
					Error(u8"<p><b>准备评估时发生了错误</b></p><p>网络通信异常</p>");
					return;
				}

				m_rep_body = req2.GetResult();
				char *m_result = strstr(m_rep_body, "\xce\xca\xbe\xed\xc6\xc0\xb9\xc0\xd2\xb3\xc3\xe6" /*"问卷评估页面"*/);
				if (m_result == NULL)
				{
					std::string err_msg = u8"<p>出现错误</p><p>很抱歉，在评估《";
					err_msg = err_msg + te[i].name + u8"》课程时出现了错误</p><p>(进入详细页面失败)</p>";
					Error(err_msg.c_str());
					return;
				}

				m_result = strstr(m_result, "<input type=\"radio\" name=\"");
				std::string rank = "&";
				while (m_result != NULL)
				{
					char *p1 = strstr(m_result + 26, "\"");
					if (p1 == NULL)
					{
						std::string err_msg = u8"<p>出现错误</p><p>很抱歉，在评估《";
						err_msg = err_msg + te[i].name + u8"》课程时出现了错误</p><p>(名称条目引号闭合失败)</p>";
						Error(err_msg.c_str());
						return;
					}
					char num[64] = { 0 };
					mid(num, m_result + 26, p1 - m_result - 26, 0);
					if (strcmp(last, num) == 0)
					{
						m_result = strstr(m_result + 26, "<input type=\"radio\" name=\"");
						continue;
					}
					else 
					{
						memset(last, 0, sizeof(last));
						strncpy(last, num, sizeof(last) - 1);
					}
					char *p2 = strstr(p1 + 1, "value=\"");
					if (p2 == NULL)
					{
						std::string err_msg = u8"<p>出现错误</p><p>很抱歉，在评估《";
						err_msg = err_msg + te[i].name + u8"》课程时出现了错误</p><p>(值条目引号开启失败)</p>";
						Error(err_msg.c_str());
						return;
					}
					char *p3 = strstr(p2 + 7, "\"");
					if (p2 == NULL)
					{
						std::string err_msg = u8"<p>出现错误</p><p>很抱歉，在评估《";
						err_msg = err_msg + te[i].name + u8"》课程时出现了错误</p><p>(值条目引号闭合失败)</p>";
						Error(err_msg.c_str());
						return;
					}

					char val[64] = { 0 };
					mid(val, p2 + 7, p3 - p2 - 7, 0);
					rank = rank + num + "=" + val + "&";

					m_result = strstr(p3, "<input type=\"radio\" name=\"");
				}

				std::string post_data = "wjbm=";
				post_data = post_data + te[i].wjbm + "&bpr=" + te[i].bpr + "&pgnr=" + te[i].pgnr + "&xumanyzg=zg&wjbz=" + rank + "zgpj=";
				post_data += zgpj;

				// 检查这门课是否需要教学评估
				CCurlTask req3;
				if (!req3.Exec(false, POST_TEACH_EVAL, CGI_HTTP_COOKIE, true, post_data.c_str()))
				{
					std::string err_msg = u8"<p>出现错误</p><p>很抱歉，在评估《";
					err_msg = err_msg + te[i].name + u8"》课程时出现了错误</p><p>网络通信异常</p>";
					Error(err_msg.c_str());
				}

				m_rep_body = req3.GetResult();
				m_result = strstr(m_rep_body, "\xb3\xc9\xb9\xa6" /*"成功"*/);
				if (m_result == NULL)
				{
					std::string err_msg = u8"<p>出现错误</p><p>很抱歉，在评估《";
					err_msg = err_msg + te[i].name + u8"》课程时出现了错误</p>";
					Error(err_msg.c_str());
					return;
				}
			}
		}
	}
	cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/TeachEval.fcgi\r\n" << GLOBAL_HEADER;
	return;
}

// 修改密码页面 (/changePassword.fcgi)
void parse_change_password()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // 还没登录就丢去登录。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
		return;
	}

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=" << APP_SUB_DIRECTORY << "/\r\n";
	cout << GLOBAL_HEADER;

	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		std::string title(u8"修改密码 - ");
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	cout << strformat( m_lpszQuery.c_str());

	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << footer.c_str();
	}
}

// 修改密码 (POST /changePassword.fcgi)
void do_change_password() //(POST /changePassword.fcgi)
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // 还没登录就丢去登录。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
		return;
	}

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0 || m_post_length > 127)
	{
		Error(u8"<p>发生错误，POST 数据长度异常</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	// 获取新密码
	std::string str_pwd = _POST(post, "mm");
	if (str_pwd.empty())
	{
		Error(u8"<p>发生错误，无法获取 POST 数据。</p>");
		return;
	}

	char pwd[4096] = { 0 };
	strncpy(pwd, str_pwd.c_str(), sizeof(pwd) - 1);
	int len = url_decode(pwd, strlen(pwd));
	char temp[4096];
	left(temp, pwd, len);
	memset(pwd, 0, sizeof(pwd));
	strncpy(pwd, temp, sizeof(pwd) - 1);

	if (len > 12 || len <= 0)
	{
		Error(u8"<p>新密码长度不能超过12个字符！</p>");
		return;
	}

	std::string GET_RET = strformat(REQ_CHANGE_PASSWORD, pwd);

	CCurlTask req;
	if (!req.Exec(false, GET_RET, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>修改密码时发生错误</b></p><p>网络通信异常</p>");
		return;
	}

	// 拉取改密结果
	char *m_rep_header = req.GetResult();
	char *pStr1 = strstr(m_rep_header, "\xb3\xc9\xb9\xa6" /*"成功"*/);
	if (pStr1 == NULL)
	{
		Error(u8"<p>密码修改失败，请确认是否输入了非法字符</p>");
		return;
	}

	EnCodeStr(pwd, pwd);
	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[2];
	memset(bind, 0, sizeof(bind));
	std::string query("UPDATE `userinfo` SET `password`=? WHERE `id`=?");

	if (stmt == NULL)
	{
		std::string Err_Msg(u8"<b>发生错误，数据库记录失败 (请使用新密码重新登录)</b><p>(");
		Err_Msg += mysql_error(&db);
		Err_Msg += ")</p>";
		Error(Err_Msg.c_str());
		return;
	}

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)pwd;
	bind[0].buffer_length = strlen(pwd);

	char id[128] = { 0 };
	get_student_id(id);
	bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[1].buffer = (void *)id;
	bind[1].buffer_length = strlen(id);

	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
		mysql_stmt_bind_param(stmt, bind) != 0 || 
		mysql_stmt_execute(stmt) != 0)
	{
		std::string Err_Msg(u8"<b>发生错误，数据库记录失败 (请使用新密码重新登录)</b><p>(");
		Err_Msg += mysql_stmt_error(stmt);
		Err_Msg += ")</p>";
		Error(Err_Msg.c_str());
		mysql_stmt_close(stmt);
		return;
	}

	mysql_stmt_close(stmt);
	student_logout();
	cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/\r\n" << GLOBAL_HEADER;
}

// 交流讨论板块
void parse_discussion()
{
	if (strlen(DISCUSSION_PAGE_CODE) == 0 && strlen(DISCUSSION_PAGE_CONTENT) == 0)
	{
		Error(u8"交流讨论功能已被关闭");
		return;
	}

	cout << GLOBAL_HEADER;

	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		std::string title = u8"交流讨论 - ";
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}

	cout << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), DISCUSSION_PAGE_CONTENT, DISCUSSION_PAGE_CODE);
	
	if (!CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		cout << footer.c_str();
	}
}