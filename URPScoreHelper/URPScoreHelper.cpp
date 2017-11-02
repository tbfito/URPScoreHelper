/*
******************************************
********** iEdon URPScoreHelper **********
**********  Model + Controller  **********
**********   Copyright © iEdon  **********
******************************************
****  Project encoding must be UTF-8  ****
******************************************
*/

#include "stdafx.h"
#include "URPScoreHelper.h"
#include "General.h"
#include "StringHelper.h"
#include "CCurlTask.h"
#include "OAuth2.h"
#include "URPRequests.h"
#include "Encrypt.h"
#include "gbkutf8.h"
#include "Admin.h"

// 请求映射入口 (FastCGI 处理循环)
void app_intro()
{
	while (FCGX_Accept_r(&request) >= 0)
	{
		LoadConfig(); // 更新配置信息
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
		isAjaxRequest = (FCGX_GetParam("HTTP_X_AJAX_REQUEST", request.envp) != NULL); // 是否是 AJAX 请求

		if (!isdbReady)
		{
			cout << "Status: 500 Internal Server Error\r\n"
				<< GLOBAL_HEADER
				<< u8"<p><b>数据库连接失败</b></p><p>" << mysql_error(&db) << "</p>";
			goto END_REQUEST;
		}

		if (CGI_REQUEST_METHOD == NULL || CGI_SCRIPT_NAME == NULL || CGI_QUERY_STRING == NULL ||
			CGI_SCRIPT_FILENAME == NULL || CGI_CONTENT_LENGTH == NULL)
		{
			cout << "Status: 500 Internal Server Error\r\n"
				<< GLOBAL_HEADER
				<< u8"<p>FastCGI 接口异常，请检查设置。</p>";
			goto END_REQUEST;
		}
		if (CGI_HTTP_COOKIE == NULL)
		{
			CGI_HTTP_COOKIE = (char *)emptystr;
		}
		if (CGI_HTTP_HOST == NULL)
		{
			CGI_HTTP_HOST = (char *)emptystr;
		}

		// 单独为 Admin 做处理。
		if (strstr(CGI_REQUEST_URI, "/admin") != NULL)
		{
			if (strcmp(CGI_SCRIPT_NAME, "/admin/login.fcgi") == 0)
			{
				if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
				{
					parse_admin_login();
					goto END_REQUEST;
				}
				if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
				{
					do_admin_login();
					goto END_REQUEST;
				}
			}

			if (strcmp(CGI_SCRIPT_NAME, "/admin/settings.fcgi") == 0)
			{
				if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
				{
					parse_admin_settings();
					goto END_REQUEST;
				}
				if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
				{
					save_admin_settings();
					goto END_REQUEST;
				}
			}

			if (strcmp(CGI_SCRIPT_NAME, "/admin/info.fcgi") == 0)
			{
				parse_admin_info();
				goto END_REQUEST;
			}

			if (strcmp(CGI_SCRIPT_NAME, "/admin/change-pass.fcgi") == 0)
			{
				if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
				{
					parse_admin_change_password();
					goto END_REQUEST;
				}
				if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
				{
					do_admin_change_password();
					goto END_REQUEST;
				}
			}

			if (strcmp(CGI_SCRIPT_NAME, "/admin/adv-card.fcgi") == 0)
			{
				if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
				{
					parse_admin_adv_card();
					goto END_REQUEST;
				}
				if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
				{
					change_admin_adv_card();
					goto END_REQUEST;
				}
			}

			if (strcmp(CGI_REQUEST_URI, "/admin/index.fcgi") == 0 || strcmp(CGI_REQUEST_URI, "/admin") == 0)
			{
				cout << "Status: 301 Moved Permanently\r\n" << "Location: " << getAppURL().c_str() << "/admin/\r\n" << GLOBAL_HEADER;
				goto END_REQUEST;
			}

			if (strcmp(CGI_REQUEST_URI, "/admin/") == 0)
			{
				parse_admin_index();
				goto END_REQUEST;
			}

			cout << "Status: 404 Not Found\r\n";
			admin_error(u8"页面未找到");
			goto END_REQUEST;

		}

		if (!isPageSrcLoadSuccess)
		{
			LoadPageSrc();
			if (!isPageSrcLoadSuccess)
			{
				cout << "Status: 500 Internal Server Error\r\n"
					<< GLOBAL_HEADER
					<< u8"<p>网页模板文件缺失或异常。</p>";
				goto END_REQUEST;
			}
		}

		// 为第三方接入做名称转换
		if (CGI_QUERY_STRING != NULL && strstr(CGI_QUERY_STRING, "3rd_party=") != NULL)
		{
			if (strstr(CGI_QUERY_STRING, "&") == NULL)
			{
				char _3rd_party[4096] = {0};
				std::cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "\r\n";
				if (strcmp(CGI_QUERY_STRING, "3rd_party=") != 0)
				{
					right(_3rd_party, CGI_QUERY_STRING, strlen(CGI_QUERY_STRING) - 10);
					std::cout << "Set-Cookie: 3rd_party=" << _3rd_party << "; max-age=1800; path=/\r\n";
				}
				std::cout << GLOBAL_HEADER;
				goto END_REQUEST;
			}
		}
		char * pStr_3rd_party = strstr(CGI_HTTP_COOKIE, "3rd_party=");
		if (CGI_HTTP_COOKIE != NULL && pStr_3rd_party != NULL)
		{
			char *pStr2 = strstr(pStr_3rd_party + 10, ";");
			char _3rd_party[4096] = { 0 };
			if (pStr2 == NULL) // 如果这条 Cookie 在最后一条
			{
				right(_3rd_party, CGI_HTTP_COOKIE, strlen(CGI_HTTP_COOKIE) - (pStr_3rd_party - CGI_HTTP_COOKIE) - 10);
			}
			else
			{
				mid(_3rd_party, pStr_3rd_party, pStr2 - pStr_3rd_party - 10, 10);
			}
			_3rd_party[1023] = '\0';
			url_decode(_3rd_party, 1024);
			strncpy(APP_NAME, _3rd_party, 1024);
		}

		// 普通请求处理
		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0) // 如果是 GET 请求
		{
			if (strcmp(CGI_SCRIPT_NAME, "/") == 0 || strcmp(CGI_SCRIPT_NAME, "/index.fcgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=logout") == 0)
				{
					student_logout();
					cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "\r\n" << GLOBAL_HEADER;
					goto END_REQUEST;
				}
				if (strcmp(CGI_QUERY_STRING, "act=requestAssoc") == 0)
				{
					bool m_need_update_cookie = false;
					std::string photo(" ");
					process_cookie(&m_need_update_cookie, photo);
					if (photo.empty())
					{
						cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/index.fcgi\r\n" << GLOBAL_HEADER;
						goto END_REQUEST;
					}
					char student_id[512] = { 0 };
					get_student_id(student_id);
					student_logout();
					EnCodeStr(student_id, student_id);
					cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/OAuth2.fcgi?stid=" << student_id << "\r\n" << GLOBAL_HEADER;
					goto END_REQUEST;
				}
				if (strcmp(CGI_REQUEST_URI, "/index.fcgi") == 0)
				{
					cout << "Status: 301 Moved Permanently\r\n" << "Location: " << getAppURL().c_str() << "\r\n" << GLOBAL_HEADER;
					goto END_REQUEST;
				}
				parse_index();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/main.fcgi") == 0)
			{
				parse_main();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2.fcgi") == 0)
			{
				OAuth2_process();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2CallBack.fcgi") == 0)
			{
				OAuth2_CallBack();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2Assoc.fcgi") == 0)
			{
				OAuth2_Association(false);
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/query.fcgi") == 0)
			{
				parse_query();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/QuickQuery.fcgi") == 0)
			{
				parse_QuickQuery_Intro();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/TeachEval.fcgi") == 0)
			{
				parse_teaching_evaluation();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/changePassword.fcgi") == 0)
			{
				parse_change_password();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/captcha.fcgi") == 0)
			{
				parse_ajax_captcha();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/avatar.fcgi") == 0)
			{
				parse_ajax_avatar();
				goto END_REQUEST;
			}
			cout << "Status: 404 Not Found\r\n";
			Error(u8"<p>找不到该页面</p>");
			goto END_REQUEST;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0) // 如果是 POST 请求
		{
			if (strcmp(CGI_SCRIPT_NAME, "/changePassword.fcgi") == 0)
			{
				do_change_password();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/query.fcgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=QuickQuery") == 0)
				{
					parse_QuickQuery_Result();
					goto END_REQUEST;
				}
				parse_query();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/main.fcgi") == 0)
			{
				parse_main();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2Assoc.fcgi") == 0)
			{
				OAuth2_Association(true);
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/TeachEval.fcgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=Evaluate") == 0)
				{
					teaching_evaluation();
					goto END_REQUEST;
				}
			}
		}
		cout << "Status: 500 Internal Server Error\r\n";
		Error(u8"<p>发生错误，未经处理的异常</p>");
		goto END_REQUEST;

		END_REQUEST:
			ZeroMemory(JSESSIONID, 256);
			FCGX_Finish_r(&request);
			//_CrtDumpMemoryLeaks();
			continue;
	}
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

	strcpy(file_root, doc_root);
	strcat(file_root, "header.fcgi");
	strcpy(HEADER_TEMPLATE_LOCATION, file_root); // HEADER_TEMPLATE_LOCATION 内存在 LoadConfig() 中做了初始化

	header = strformat(ReadTextFileToMem(file_root).c_str(), "%s", SECONDARY_TITLE, APP_KEYWORDS, APP_DESCRIPTION);

	strcpy(file_root, doc_root);
	strcat(file_root, "footer.fcgi");
	strcpy(FOOTER_TEMPLATE_LOCATION, file_root); // FOOTER_TEMPLATE_LOCATION 内存在 LoadConfig() 中做了初始化

	footer = strformat(ReadTextFileToMem(file_root).c_str(), APP_NAME, FOOTER_TEXT, SOFTWARE_NAME, ANALYSIS_CODE);

	strcpy(file_root, doc_root);
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
		HEADER_TEMPLATE_LOCATION = (char *)malloc(10240);
		memset(HEADER_TEMPLATE_LOCATION, 0, 10240);
	}
	if (FOOTER_TEMPLATE_LOCATION == NULL)
	{
		FOOTER_TEMPLATE_LOCATION = (char *)malloc(10240);
		memset(FOOTER_TEMPLATE_LOCATION, 0, 10240);
	}

	if (!isdbReady)
	{
		if (!mysql_real_connect(&db, MYSQL_HOST, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DBNAME, atoi(MYSQL_PORT_NUMBER), NULL, 0))
		{
			isdbReady = false;
			fprintf(stderr, "Failed to connect to database: Error: %s\n", mysql_error(&db));
			return; // unlucky..
		}
		else
		{
			isdbReady = true;
		}
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

	GetSettings("CURL_TIMEOUT", lpvBuffer);
	CURL_TIMEOUT = atoi(lpvBuffer);
	if (CURL_TIMEOUT <= 0)
		CURL_TIMEOUT = 2;
	memset(lpvBuffer, 0, 10240);

	GetSettings("CURL_USE_PROXY", lpvBuffer);
	CURL_USE_PROXY = (atoi(lpvBuffer) == 1);

	memset(lpvBuffer, 0, 10240);
	GetSettings("ENABLE_QUICK_QUERY", lpvBuffer);
	ENABLE_QUICK_QUERY = (atoi(lpvBuffer) == 1);

	free(lpvBuffer);
	
	std::string query("CREATE TABLE IF NOT EXISTS `UserInfo` (`id` char(36) NOT NULL,`password` char(36) NOT NULL,`name` varchar(36) DEFAULT NULL,`openid` varchar(1024) DEFAULT NULL,`OAuth_name` varchar(1024) DEFAULT NULL,`OAuth_avatar` varchar(4096) DEFAULT NULL,`lastlogin` datetime DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8;");
	if (mysql_query(&db, query.c_str()) != 0)
	{
		return;
	}

	query = "CREATE TABLE IF NOT EXISTS `Settings` (`name` varchar(254) NOT NULL,`value` varchar(10240) NOT NULL,PRIMARY KEY (`name`)) ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	if (mysql_query(&db, query.c_str()) != 0)
	{
		return;
	}

	// 如果数据库没有下面配置，则自动增加并写入默认值以确保首次能够正常运行。
	AddSettings("QueryCounter", "0");
	AddSettings("SERVER_URL", "http://0.0.0.0");
	AddSettings("USER_AGENT", SOFTWARE_NAME);
	AddSettings("OAUTH2_APPID", "");
	AddSettings("OAUTH2_SECRET", "");
	AddSettings("CURL_PROXY_URL", "");
	AddSettings("APP_NAME", SOFTWARE_NAME);
	AddSettings("CURL_TIMEOUT", "2");
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
	AddSettings("ENABLE_QUICK_QUERY", "1");
}

// 更新用户数量、查询计数器
void UpdateCounter()
{
	g_QueryCounter = 0;
	g_users = 0;

	// 获取多少用户使用了我们的服务 :)
	std::string query("SELECT `value` FROM `Settings` WHERE name='QueryCounter';");
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (mysql_query(&db, query.c_str()) != 0)
	{
		return;
	}
	result = mysql_store_result(&db);
	if (mysql_num_rows(result))
	{
		while ((row = mysql_fetch_row(result)))
		{
			if (row[0])
			{
				char query_counts[64] = { 0 };
				sprintf(query_counts, "%s", row[0]);
				g_QueryCounter = atoi(query_counts);
			}
			break;
		}
	}
	mysql_free_result(result);
	memset(&row, 0, sizeof(MYSQL_ROW));

	query = "SELECT COUNT(*) FROM `UserInfo`;";
	if (mysql_query(&db, query.c_str()) != 0)
	{
		return;
	}
	result = mysql_store_result(&db);
	if (mysql_num_rows(result))
	{
		while ((row = mysql_fetch_row(result)))
		{
			if (row[0])
			{
				char query_counts[64] = { 0 };
				sprintf(query_counts, "%s", row[0]);
				g_users = atoi(query_counts);
			}
			break;
		}
	}
	mysql_free_result(result);
}

// 读取数据库设置表中的内容(注意value的内存分配)
bool GetSettings(const char *name, char *value)
{
	// 获取多少用户使用了我们的服务 :)
	std::string query("SELECT `value` FROM `Settings` WHERE name='");
	query += name;
	query += "';";

	MYSQL_RES *result;
	MYSQL_ROW row;
	if (mysql_query(&db, query.c_str()) != 0)
	{
		strcpy(value, "(null)");
		return false;
	}
	result = mysql_store_result(&db);
	if (mysql_num_rows(result))
	{
		while ((row = mysql_fetch_row(result)))
		{
			if (row[0])
			{
				sprintf(value, "%s", row[0]);
			}
			break;
		}
	}
	else
	{
		strcpy(value, "(null)");
	}
	mysql_free_result(result);
	return true;
}

// 向数据库设置表中增添配置项
bool AddSettings(const char *name, const char *value)
{
	std::string query("INSERT IGNORE INTO `Settings` (`name`, `value`) VALUES ('");
	query =  query + name + "', '" + value + "');";
	if (mysql_query(&db, query.c_str()) != 0)
	{
		return false;
	}
	return true;
}

// 置查询计数器
void SetQueryCounter(int current_counts)
{
	std::string query("UPDATE `Settings` SET value='");
	char counts[128] = { 0 };
	sprintf(counts, "%d", current_counts);
	query += counts;
	query += "' WHERE name='QueryCounter';";

	if (mysql_query(&db, query.c_str()) != 0)
	{
		return;
	}
}

// 处理 Cookie、照片(p_photo_uri 为空代表不要照片, 随便设置内容不为空则会向里面写入照片数据)
int process_cookie(bool *p_need_update_cookie, std::string & p_photo_uri)
{
	char *pStr = strstr(CGI_HTTP_COOKIE, "JSESSIONID=");
	if (pStr != NULL) // 如果客户端已经拿到 JSESSIONID，看看原 Cookie 是否过期、有效（即服务器是否设置了新 Cookie）
	{
		CCurlTask req;
		if (!req.Exec(true, REQUEST_HOME_PAGE, CGI_HTTP_COOKIE))
		{
			p_photo_uri.erase();
			return -1;
		}
		char *m_rep_header = req.GetResult();

		char *pStr1 = strstr(m_rep_header, "JSESSIONID=");
		if (pStr1 != NULL)
		{
			char *pStr2 = strstr(pStr1 + 11, ";");
			if (pStr2 == NULL)
			{
				Error(u8"<p>无法获取 Session ID</p><p>Cookie 结尾失败</p>");
				p_photo_uri.erase();
				return -1;
			}
			mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // 成功获得新 Session ID。
			*p_need_update_cookie = true;
			return -1;
		}
		else // 如果 Cookie 还能用，就获取它。
		{
			char *pStr2 = strstr(pStr + 11, ";");
			if (pStr2 == NULL) // 如果这条 Cookie 在最后一条
			{
				right(JSESSIONID, CGI_HTTP_COOKIE, strlen(CGI_HTTP_COOKIE) - (pStr - CGI_HTTP_COOKIE) - 11);
			}
			else
			{
				mid(JSESSIONID, pStr, pStr2 - pStr - 11, 11);
			}
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
		char *m_rep_header = req.GetResult();
		// 获取 Session ID。
		char *pStr1 = strstr(m_rep_header, "JSESSIONID=");
		if (pStr1 == NULL)
		{
			Error(u8"<p>无法获取 Session ID</p><p>Cookie 标头失败</p>");
			p_photo_uri.erase();
			return -1;
		}
		char *pStr2 = strstr(pStr1 + 11, ";");
		if (pStr2 == NULL)
		{
			Error(u8"<p>无法获取 Session ID</p><p>Cookie 结尾失败</p>");
			p_photo_uri.erase();
			return -1;
		}

		mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // 成功获得 Session ID。
		*p_need_update_cookie = true;
	}

	if (p_photo_uri.empty())
		return -1;  // p_photo_uri 指定了 NULL 代表不需要照片。

	// 看看登录没
	std::string Jsess ("JSESSIONID=");
	Jsess += JSESSIONID;

	CCurlTask req;
	if (!req.Exec(false, REQUEST_PHOTO, Jsess))
	{
		p_photo_uri.erase();
		return -1;
	}
	char *m_photo = req.GetResult();
	if (strstr(m_photo, "\xb5\xc7\xc2\xbc" /*登录*/) == NULL)
	{
		int m_photoLength = req.GetLength();

		char *m_base64 = (char *)malloc(m_photoLength * 3 + 1);
		ZeroMemory(m_base64, m_photoLength * 3 + 1);
		base64_encode((const unsigned char *)m_photo, m_base64, m_photoLength);
		char *m_PhotoDataURI = (char *)malloc(m_photoLength * 3 + 1 + 24);
		ZeroMemory(m_PhotoDataURI, m_photoLength * 3 + 1 + 24);
		strcpy(m_PhotoDataURI, "data:image/jpg;base64,");
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
			Error(u8"<p><b>发生错误，POST 数据长度异常</b></p><p>帐号或密码输入有问题哦，请重试</p>");
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

		// 获取学号
		char *pStr1 = strstr(m_post_data, "xh=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error(u8"<p>无法获取学号信息。</p>");
			return;
		}
		char *pStr2 = strstr(pStr1 + 3, "&");
		char m_xuehao[128] = { 0 };
		mid(m_xuehao, pStr1, pStr2 - pStr1 - 3, 3);
		pStr1 = NULL;
		pStr2 = NULL;

		// 获取密码
		pStr1 = strstr(m_post_data, "mm=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error(u8"<p>无法获取密码信息。</p>");
			return;
		}
		pStr2 = strstr(pStr1 + 3, "&");
		char m_password[128] = { 0 };
		mid(m_password, pStr1, pStr2 - pStr1 - 3, 3);
		pStr1 = NULL;
		pStr2 = NULL;

		// 获取验证码
		pStr1 = strstr(m_post_data, "yzm=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error(u8"<p>无法获取验证码信息。</p>");
			return;
		}
		char m_captcha[128] = { 0 };
		right(m_captcha, pStr1 + 4, 4);

		if (!student_login(m_xuehao, m_password, m_captcha))
		{
			// 其余资源清理已在学生登录里面做过了。
			free(m_post_data);
			return;
		}
		free(m_post_data);
		char m_xuehaoe[1024] = { 0 };
		strcpy(m_xuehaoe, m_xuehao);
		char m_passworde[1024] = { 0 };
		strcpy(m_passworde, m_password);
		EnCodeStr(m_xuehaoe, m_xuehaoe);
		EnCodeStr(m_passworde, m_passworde);
		std::string token(m_xuehaoe);
		token += "X";
		token += m_passworde;
		char token_e[4096] = { 0 };
		strcpy(token_e, token.c_str());
		EnCodeStr(token_e, token_e);
		cout << "Set-Cookie: token=" << token_e << "; path=/\r\n";
	}

	// 读入主页面文件
	std::string m_lpszHomepage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	char m_student_name[512] = {0};
	char m_student_id[512] = { 0 };
	char m_avatar_url[512] = { 0 };
	get_student_id(m_student_id);
	if (!GetOAuthUserInfo(m_student_id, m_student_name, m_avatar_url))
	{
		memset(m_student_name, 0, 512);
		get_student_name(m_student_name);
	}

	// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
	std::string query("SELECT `openid` FROM `UserInfo` WHERE id='");
	query.append(m_student_id);
	query.append("';");

	MYSQL_RES *result;
	MYSQL_ROW row;
	char openid[512] = { 0 };

	if (mysql_query(&db, query.c_str()) != 0)
	{
		return;
	}
	result = mysql_store_result(&db);
	if (mysql_num_rows(result))
	{
		while ((row = mysql_fetch_row(result)))
		{
			if (row[0])
			{
				sprintf(openid, "%s", row[0]);
			}
			break;
		}
	}
	mysql_free_result(result);

	cout << GLOBAL_HEADER;

	if (!isAjaxRequest)
	{
		std::string title(m_student_name);
		title += u8" - ";
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	std::string OutputAd;
	if (strlen(CARD_AD_BANNER_1_IMG) > 0)
	{
		char AdUrl[4096] = { 0 };
		sprintf(AdUrl, CARD_AD_BANNER_HTML, CARD_AD_BANNER_1_URL, CARD_AD_BANNER_1_IMG);
		OutputAd += AdUrl;
	}
	if (strlen(CARD_AD_BANNER_2_IMG) > 0)
	{
		char AdUrl[4096] = { 0 };
		sprintf(AdUrl, CARD_AD_BANNER_HTML, CARD_AD_BANNER_2_URL, CARD_AD_BANNER_2_IMG);
		OutputAd += AdUrl;
	}
	if (strlen(openid) == 0)
	{
		cout << strformat(m_lpszHomepage.c_str(), APP_NAME, m_student_name, m_student_id, 
							ASSOC_LINK_HTML, OutputAd.c_str());
	}
	else
	{
		cout << strformat(m_lpszHomepage.c_str(), APP_NAME, m_student_name, m_student_id,
							strformat(RLS_ASSOC_LINK_HTML, m_student_id).c_str(), OutputAd.c_str());
	}
	if (!isAjaxRequest)
	{
		cout << footer.c_str();
	}
}

// 处理主页面请求 (GET / /index.fcgi)
int parse_index()
{
	// 如果是QQ登录回来，则自动填充帐号密码。
	char *m_xh = NULL;
	char *m_mm = NULL;
	char *token_xh = NULL;
	char *token_mm = NULL;
	char *pStr1 = strstr((char *)CGI_QUERY_STRING, "stid=");
	if (pStr1 != NULL)
	{
		char *pStr2 = strstr(pStr1 + 5, "&");
		char *id = new char[strlen(CGI_QUERY_STRING)];
		if (pStr2 == NULL)
		{
			right(id, pStr1 + 5, strlen(CGI_QUERY_STRING) - 5);
		}
		else
		{
			mid(id, pStr1 + 5, pStr2 - pStr1 - 5, 0);
		}
		DeCodeStr(id);
		m_xh = id;
		pStr1 = strstr((char *)CGI_QUERY_STRING, "pass=");
		if (pStr1 != NULL)
		{
			char *pStr2 = strstr(pStr1 + 5, "&");
			char *pass = new char[strlen(CGI_QUERY_STRING)];
			if (pStr2 == NULL)
			{
				right(pass, pStr1 + 5, strlen(CGI_QUERY_STRING) - 5);
			}
			else
			{
				mid(pass, pStr1 + 5, pStr2 - pStr1 - 5, 0);
			}
			DeCodeStr(pass);
			m_mm = pass;
		}
	}
	else
	{
		char token[4096] = { 0 };
		char *pStr = strstr(CGI_HTTP_COOKIE, "token=");
		if (pStr != NULL)
		{
			char *pStr2 = strstr(pStr + 6, ";");
			if (pStr2 == NULL) // 如果这条 Cookie 在最后一条
			{
				right(token, CGI_HTTP_COOKIE, strlen(CGI_HTTP_COOKIE) - (pStr - CGI_HTTP_COOKIE) - 6);
			}
			else
			{
				mid(token, pStr, pStr2 - pStr - 6, 6);
			}
		}
		DeCodeStr(token);
		token_xh = (char *)malloc(1024);
		token_mm = (char *)malloc(1024);
		if (sscanf(token, "%[^X]%s", token_xh, token_mm) != 2)
		{
			free(token_xh);
			free(token_mm);
			token_xh = NULL;
			token_mm = NULL;
		}
		else
		{
			DeCodeStr(token_xh);
			DeCodeStr(token_mm);
		}
	}

	// 读入主页面文件
	std::string m_lpszHomepage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	cout << GLOBAL_HEADER;
	if (!isAjaxRequest)
	{
		cout << strformat(header.c_str(), APP_NAME);
	}

	if (m_xh == NULL || m_mm == NULL)
	{
		if (token_xh != NULL && token_mm != NULL)
		{
			cout << strformat(m_lpszHomepage.c_str(), APP_NAME, g_users, g_QueryCounter,
				u8"输入你的教务系统帐号来登录吧 :)", token_xh, token_mm, u8"登录", " col-50",
				OAUTH2_LOGIN_HTML, ENABLE_QUICK_QUERY ? QUICKQUERY_HTML : "");
		}
		else
		{
			cout << strformat(m_lpszHomepage.c_str(), APP_NAME, g_users, g_QueryCounter,
				u8"输入你的教务系统帐号来登录吧 :)", "", "", u8"登录", " col-50",
				OAUTH2_LOGIN_HTML, ENABLE_QUICK_QUERY ? QUICKQUERY_HTML : "");
		}
	}
	else 
	{
		cout << strformat( m_lpszHomepage.c_str(), APP_NAME, g_users, g_QueryCounter,
						u8"QQ登录成功，输入验证码继续吧 :)", m_xh, m_mm, u8"继续", "", "", "");
	}
	if (!isAjaxRequest)
	{
		cout << footer.c_str();
	}
	if (m_xh != NULL)
		delete[]m_xh;
	if (m_mm != NULL)
		delete[]m_mm;
	if (token_xh != NULL)
		free(token_xh);
	if (token_mm != NULL)
		free(token_mm);
	return 0;
}

// 处理验证码 Ajax 请求
void parse_ajax_captcha() //(AJAX: GET /captcha.fcgi)
{
	cout << "Cache-Control: no-cache\r\nPragma: no-cache\r\nExpires: Thu, 16 Oct 1997 00:00:00 GMT\r\nContent-Type: text/plain; charset=utf-8\r\n";
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";

	if (!m_photo.empty() && !m_need_update_cookie) // 登录了就通报已经登录
	{
		cout << "\r\nLOGGED-IN";
		return;
	}

	// 置随机数种子，并取得一个随机数，用于获取验证码。
	srand((int)time(0));
	int m_rand = rand();
	char Captcha[256] = { 0 };
	sprintf(Captcha, REQUEST_CAPTCHA, m_rand);

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
		cout << "\r\nREQUEST-FAILED";
		return;
	}
	char *m_rep_body = req.GetResult();

	int m_CaptchaLength = req.GetLength(); // 验证码图片的大小

	// 将验证码转化为 base64 编码后的 DataURL，浏览器直接显示，供用户查看。
	char *m_base64 = new char[m_CaptchaLength * 2 + 1];
	base64_encode((const unsigned char *)m_rep_body, m_base64, m_CaptchaLength);
	char *m_DataURL = new char[m_CaptchaLength * 2 + 24];;
	strcpy(m_DataURL, "data:image/jpg;base64,");
	strcat(m_DataURL, m_base64);

	cout << "\r\n";
	cout << m_DataURL;

	delete[]m_base64;
	delete[]m_DataURL;
}

// 处理头像 Ajax 请求
void parse_ajax_avatar()
{
	cout << "Content-Type: text/plain; charset=utf-8\r\n";
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";

	if (m_photo.empty() || m_need_update_cookie)
	{
		cout << "\r\nLOGGED-OUT";
		return;
	}
	cout << "\r\n";
	char m_student_id[512] = { 0 };
	char m_avatar_url[512] = { 0 };
	char m_student_name[512] = { 0 };
	get_student_id(m_student_id);
	if (!GetOAuthUserInfo(m_student_id, m_student_name, m_avatar_url))
	{
		cout << m_photo.c_str();
	}
	else
	{
		cout << m_avatar_url;
	}
}

// 处理查询页面请求 (GET /query.fcgi)
int parse_query()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // 还没登陆就丢去登陆。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/index.fcgi\r\n" << GLOBAL_HEADER;
		return 0;
	}

	// 开始查分(本学期)。
	CCurlTask req;
	if (!req.Exec(false, REQUEST_QUERY_SCORE, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>接收数据失败</b></p><p>curl 操作失败</p>");
		student_logout();
		return -1;
	}

	// 优化接收结果，显示查询页面
	parse_friendly_score(req.GetResultString());
	return 0;
}

// 输出分数页面
void parse_friendly_score(std::string & p_strlpszScore)
{
	char m_Student[128] = { 0 };
	get_student_name(m_Student);

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	char *p_lpszScore = (char *)malloc(p_strlpszScore.length() + 1);
	strcpy(p_lpszScore, p_strlpszScore.c_str());

	char *m_query_not_reg = strstr(p_lpszScore, "\xc3\xbb\xd3\xd0\xd7\xa2\xb2\xe1" /*没有注册*/);
	if (m_query_not_reg != NULL) // 如果还没有电子注册
	{
		/*std::string m_original_str (u8"<p><b>亲爱的%s，每学期首次使用需要电子注册</b></p><p>不注册的话，是查不了信息的哦！</p><p>我可以施展法术，\
<b>一键帮你在教务系统注册哦~</b></p><p>--&gt; 点按下方按钮，自动注册，直达查分界面 :P &lt;--</p>\
<div class=\"weui-msg__opr-area\"><p class=\"weui-btn-area\"><a style=\"color:#fff\" href=\"query.fcgi?act=system_registration\" class=\"weui-btn weui-btn_primary\">【点我】一键注册</a></p></div>");
		m_original_str = strformat(m_original_str.c_str(), m_Student);
		Error(m_original_str.c_str());
		*/
		if (system_registration() != 0)
		{
			free(p_lpszScore);
			return;
		}
	}
	if (strcmp(CGI_QUERY_STRING, "order=tests") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, GET_SMALL_TEST_SCORE, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>curl 操作失败</p>");
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
			Error(u8"<p><b>从服务器拉取分数失败。(BeginOfTable)</b></p><p>教务君可能月线繁忙，请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		m_result += 93;
		char *m_prep = (char *)malloc(req.GetLength());
		replace_string(m_result, "&nbsp;", "");
		replace_string(m_result, "\t", "");
		replace_string(m_result, "\r", "");
		replace_string(m_result, "\n", "");
		strcpy(m_prep, "<div id=\"list_page\">");
		char *m_end_body = strstr(m_result, "</table>");
		if (m_result == NULL)
		{
			free(m_prep);
			Error(u8"<p>从服务器拉取分数失败。(EndOfBodyNotFound)</p>");
			return;
		}
		m_result -= 93;
		cout << GLOBAL_HEADER;
		char m_before[512] = { 0 };
		sprintf(m_before, "<a name=\"qb_731\"></a><table width=\"100%%\" border=\"0\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td class=\"Linetop\"></td></tr></tbody></table><table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\"><tbody><tr><td width=\"100%%\"><table border=\"0\" align=\"left\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td>&nbsp;</td><td valign=\"middle\">&nbsp;<b>%s</b> &nbsp;</td></tr></tbody></table></td></tr></tbody></table>", u8"成绩清单（月考/期中/补考/缓考/清考）");
		*(m_end_body + 8) = '<';
		*(m_end_body + 9) = '/';
		*(m_end_body + 10) = 'd';
		*(m_end_body + 11) = 'i';
		*(m_end_body + 12) = 'v';
		*(m_end_body + 13) = '>';
		*(m_end_body + 14) = '\0';

		strcat(m_prep, m_before);
		strcat(m_prep, m_result);

		if (!isAjaxRequest)
		{
			std::string title(m_Student);
			title += u8"的考试成绩 - ";
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		if (!isAjaxRequest)
		{
			cout << footer.c_str();
		}

		SetQueryCounter(++g_QueryCounter);

		free(m_prep);
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=passed") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_QBINFO, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>curl 操作失败</p>");
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
			Error(u8"<p><b>从服务器拉取分数失败。(BeginOfRet)</b></p><p>教务君可能月线繁忙，请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		m_result += 92;
		char *m_prep = (char *)malloc(req.GetLength());
		replace_string(m_result, "&nbsp;", "");
		replace_string(m_result, "\t", "");
		replace_string(m_result, "\r", "");
		replace_string(m_result, "\n", "");
		strcpy(m_prep, "<div id=\"list_page\">");
		char *m_end_body = strstr(m_result, "</body>");
		if (m_result == NULL)
		{
			free(m_prep);
			Error(u8"<p>从服务器拉取分数失败。(EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		if (!isAjaxRequest)
		{
			std::string title(m_Student);
			title += u8"的通过科目 - ";
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		if (!isAjaxRequest)
		{
			cout << footer.c_str();
		}

		SetQueryCounter(++g_QueryCounter);

		free(m_prep);
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=byplan") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_PLAN, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>curl 操作失败</p>");
			return;
		}
		
		char *m_rep_body = req.GetResult();

		char *m_result = strstr(m_rep_body, "<body leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\" style=\"overflow:auto;\">");
		if (m_result == NULL)
		{
			Error(u8"<p><b>从服务器拉取分数失败。(BeginOfRet)</b></p><p>教务君可能月线繁忙，请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
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
				char *u8strtmp = (char *)malloc(strlen(zymc) * 3 + 1);
				unsigned int u8len = 0;
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
						char *u8strtmp = (char *)malloc(strlen(info) * 3 + 1);
						unsigned int u8len = 0;
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
						strcpy(m_chengji, temp);
					}
					flags++;
					break;
				}
				case weiguoyuanyin:
				{
					mid(m_weiguoyuanyin, p1 + 18, p2 - p1 - 19, 0);
					//Trim(m_weiguoyuanyin);
					char *m_StrTmp = new char[8192];
					sprintf(m_StrTmp, SCORE_TEMPLATE_BY_PLAN,
						(f_chengji < 60 && f_xuefen != 0) ? "background-color:rgba(255,0,0,0.5);color:#fff" : "",
						m_kechengming, m_shuxing, m_chengji, m_xuefen, f_jidian, m_weiguoyuanyin);

					char *u8strtmp = (char *)malloc(strlen(m_StrTmp) * 3 + 1);
					unsigned int u8len = 0;
					gbk_to_utf8(m_StrTmp, (unsigned int)strlen(m_StrTmp), &u8strtmp, &u8len);

					m_Output.append(u8strtmp);
					free(u8strtmp);
					delete[]m_StrTmp;
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
			char *m_StrTmp = new char[strlen(SCORE_TEMPLATE_BY_PLAN) + 25 + 64 + 1];
			sprintf(m_StrTmp, SCORE_TEMPLATE_BY_PLAN, "", u8"还没有任何成绩", "", "", "", 0.0, "");
			m_Output.append(m_StrTmp);
			delete[]m_StrTmp;
		}

		m_Output.append(AFTER_TEMPLATE);

		cout << GLOBAL_HEADER;

		if (!isAjaxRequest)
		{
			std::string title(m_Student);
			title += u8"的专业方案 - ";
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat(m_lpszQuery.c_str(), m_Student, m_Output.c_str());
		if (!isAjaxRequest)
		{
			cout << footer.c_str();
		}
		SetQueryCounter(++g_QueryCounter);
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=failed") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_FAILED, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>curl 操作失败</p>");
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
			Error(u8"<p><b>从服务器拉取分数失败。(BeginOfRet)</b></p><p>教务君可能月线繁忙，请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		m_result = strstr(m_result + 92, "<table width=\"100%\"  border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\">");
		if (m_result == NULL)
		{
			Error(u8"<p><b>从服务器拉取分数失败。(MidOfRet-Table)</b></p><p>教务君可能月线繁忙，请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		char *m_prep = (char *)malloc(req.GetLength());
		replace_string(m_result, "&nbsp;", "");
		replace_string(m_result, "\t", "");
		replace_string(m_result, "\r", "");
		replace_string(m_result, "\n", "");
		strcpy(m_prep, "<div id=\"list_page\">");
		char *m_end_body = strstr(m_result, "</body>");
		if (m_result == NULL)
		{
			free(m_prep);
			Error(u8"<p>从服务器拉取分数失败。(EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		if (!isAjaxRequest)
		{
			std::string title(m_Student);
			title += u8"的未通过科目 - ";
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		if (!isAjaxRequest)
		{
			cout << footer.c_str();
		}

		SetQueryCounter(++g_QueryCounter);

		free(m_prep);
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=schedule") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, REQUEST_SCHEDULE, CGI_HTTP_COOKIE))
		{
			Error(u8"<p><b>接收数据失败</b></p><p>curl 操作失败</p>");
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

		strcpy(m_prep, "<div id=\"list_page\">");
		char *m_end_body = strstr(m_result, "</table>");
		if (m_result == NULL)
		{
			free(m_prep);
			Error(u8"<p>从服务器拉取课程表失败。(EndOfBodyNotFound)</p>");
			return;
		}
		m_result -= 81;
		cout << GLOBAL_HEADER;
		char m_before[512] = { 0 };
		sprintf(m_before, "<a name=\"qb_731\"></a><table width=\"100%%\" border=\"0\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td class=\"Linetop\"></td></tr></tbody></table><table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\"><tbody><tr><td width=\"100%%\"><table border=\"0\" align=\"left\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td>&nbsp;</td><td valign=\"middle\">&nbsp;<b>%s</b> &nbsp;</td></tr></tbody></table></td></tr></tbody></table>", u8"我的课程表 / 选课结果");
		*(m_end_body + 26) = '<';
		*(m_end_body + 27) = '/';
		*(m_end_body + 28) = 'd';
		*(m_end_body + 29) = 'i';
		*(m_end_body + 30) = 'v';
		*(m_end_body + 31) = '>';
		*(m_end_body + 32) = '\0';

		strcat(m_prep, m_before);
		strcat(m_prep, m_result);

		if (!isAjaxRequest)
		{
			std::string title(u8"本学期课程表 - ");
			title += APP_NAME;
			cout << strformat(header.c_str(), title.c_str());
		}
		cout << strformat(m_lpszQuery.c_str(), m_Student, m_prep);
		if (!isAjaxRequest)
		{
			cout << footer.c_str();
		}

		SetQueryCounter(++g_QueryCounter);
		free(m_prep);
		return;
	}

	// 定位到第一项成绩
	char *pStr1 = strstr(p_lpszScore,"<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
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
		//if (atof(m_subXuefen) == 0) sprintf(m_subXuefen, "暂无数据");

		pStr2 = pStr3;
		pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuigaofen[128] = { 0 };
		mid(m_subzuigaofen, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subzuigaofen);
		//if (atof(m_subzuigaofen) == 0) sprintf(m_subzuigaofen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuidifen[128] = { 0 };
		mid(m_subzuidifen, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subzuidifen);
		//if (atof(m_subzuidifen) == 0) sprintf(m_subzuidifen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subjunfen[128] = { 0 };
		mid(m_subjunfen, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subjunfen);
		//if (atof(m_subjunfen) == 0) sprintf(m_subjunfen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subchengji[256] = { 0 };
		mid(m_subchengji, pStr2, pStr3 - pStr2 - 19, 19);
		//Trim(m_subchengji);
		if (strstr(m_subchengji, "\xd3\xc5\xd0\xe3" /*优秀*/) != NULL)
		{
			strcpy(m_subchengji,"95");
		}
		if (strstr(m_subchengji, "\xc1\xbc\xba\xc3" /*良好*/) != NULL)
		{
			strcpy(m_subchengji, "85");
		}
		if (strstr(m_subchengji, "\xd6\xd0\xb5\xc8" /*中等*/) != NULL)
		{
			strcpy(m_subchengji, "75");
		}
		if (strstr(m_subchengji, "\xbc\xb0\xb8\xf1" /*及格*/) != NULL)
		{
			if (atoi(m_subzuidifen) > 60)
			{
				strcpy(m_subchengji, m_subzuidifen);
			}
			else
			{
				strcpy(m_subchengji, "60");
			}
			
		}
		if (strstr(m_subchengji, "\xb2\xbb\xbc\xb0\xb8\xf1" /*不及格*/) != NULL)
		{
				strcpy(m_subchengji, "55");
				isPassed = false;
		}
		//if (atoi(m_subchengji) == 0) strcpy(m_subchengji, "暂无数据");
		if (atof(m_subchengji) < 60) 
		{

			char m_completecj[256] = "<b style=\"color:#f6383a\">";
			strcat(m_completecj, m_subchengji);
			strcat(m_completecj, "</b>");
			ZeroMemory(m_subchengji, 256);
			strcpy(m_subchengji, m_completecj);
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
		//if (atof(m_submingci) == 0) sprintf(m_submingci, "暂无数据");

		// （分数x学分）全都加起来/总学分 = 加权分，排除体育和课程设计
		float m_xuefen = atof(m_subXuefen);
		float m_chengji = atof(m_subchengji);
		float m_kcxfjd = m_xuefen * cj2jd(m_chengji);
		if (strstr(m_subName, "\xcc\xe5\xd3\xfd" /*体育*/) == NULL && strstr(m_subName, "\xbe\xfc\xca\xc2\xd1\xb5\xc1\xb7" /*军事训练*/) == NULL
			/* && strstr(m_subName, "\xca\xb5\xbc\xf9" [实践]) == NULL */)
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

		char *m_StrTmp = new char[strlen(SCORE_TEMPLATE) + 50 + strlen(m_subName) + strlen(m_subchengji) + strlen(m_subjunfen) + strlen(m_subzuigaofen) + strlen(m_subzuidifen) + strlen(m_submingci) + strlen(m_subXuefen) + 16 + 1];
		sprintf(m_StrTmp, SCORE_TEMPLATE, isPassed ? "": "background-color:rgba(255,0,0,0.5);color:#fff", m_subName, m_subchengji, m_subjunfen, m_subzuigaofen, m_subzuidifen,
			m_submingci, m_subXuefen, m_kcxfjd);

		char *u8strtmp = (char *)malloc(strlen(m_StrTmp) * 3 + 1);
		unsigned int u8len = 0;
		gbk_to_utf8(m_StrTmp, (unsigned int)strlen(m_StrTmp), &u8strtmp, &u8len);

		m_Output.append(u8strtmp);

		free(u8strtmp);
		delete[]m_StrTmp;
		m_success = true; // 查到一个算一个
		pStr1 = strstr(pStr3, "<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
	}

	if (hasChengji == false)
	{
		char *m_StrTmp = new char[strlen(SCORE_TEMPLATE) + 25 + 64 + 1];
		sprintf(m_StrTmp, SCORE_TEMPLATE, "", u8"本学期还未出成绩", "", "", "", "", "", "", 0.0);
		m_Output.append(m_StrTmp);
		delete[]m_StrTmp;
	}

	// 假如发生了错误
	if (!m_success) 
	{
		free(p_lpszScore);
		Error(u8"<p>现在还没有出成绩或发生了错误噢</p>");
		return;
	}

	m_Output.append(AFTER_TEMPLATE);

	// 填充返回页面
	//if (m_Total_pointsxxuefen != 0 || m_Total_xuefen != 0)
	//{
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
		char m_jiaquanfen[1024] = { 0 };
		sprintf(m_jiaquanfen, u8"<div id=\"i_total\"><p>加权平均分 / GPA(平均绩点)：</p><center>%.1f&nbsp;&nbsp;&nbsp;&nbsp;%.2f</center></div>",
				jiaquan, gpa);
		m_Output.insert(0, m_jiaquanfen);
	//}

	cout << GLOBAL_HEADER;

	if (!isAjaxRequest)
	{
		std::string title(m_Student);
		title += u8"的本学期成绩 - ";
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	cout << strformat( m_lpszQuery.c_str(), m_Student, m_Output.c_str());
	if (!isAjaxRequest)
	{
		cout << footer.c_str();
	}

	SetQueryCounter(++g_QueryCounter);
	free(p_lpszScore);
}

// 获取学生姓名
void get_student_name(char *p_lpszBuffer)
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}

	CCurlTask req;
	if (!req.Exec(false, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE))
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}
	char *m_rep_header = req.GetResult();
	// 拉取学生姓名
	char *pStr1 = strstr(m_rep_header, "\x3c\x74\x64\x20\x63\x6c\x61\x73\x73\x3d\x22\x66\x69\x65\x6c\x64\x4e\x61\x6d\x65\x22\x3e\xd0\xd5\xc3\xfb\x3a\x26\x6e\x62\x73\x70\x3b\x3c\x2f\x74\x64\x3e" /*"<td class=\"fieldName\">姓名:&nbsp;</td>"*/);
	if (pStr1 == NULL)
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}
	pStr1 = strstr(pStr1 + 39, "<td>");
	if (pStr1 == NULL)
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}
	char *pStr2 = strstr(pStr1 + 5,"</td>");
	if (pStr2 == NULL)
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}
	mid(p_lpszBuffer, pStr1, pStr2 - pStr1 - 4, 4);
	char *temp = (char *)malloc(512);
	unsigned int u8len = 0;
	gbk_to_utf8(p_lpszBuffer, (unsigned int)strlen(p_lpszBuffer), &temp, &u8len);
	strcpy(p_lpszBuffer, temp);
	free(temp);
}

// 获取学生帐号
void get_student_id(char *p_lpszBuffer)
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}

	CCurlTask req;
	if (!req.Exec(false, REQUEST_TOP, CGI_HTTP_COOKIE))
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}

	char *m_rep_header = req.GetResult();
	// 拉取学生姓名
	char *pStr1 = strstr(m_rep_header, "\xb5\xb1\xc7\xb0\xd3\xc3\xbb\xa7\x3a" /*"当前用户:"*/);
	if (pStr1 == NULL)
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}
	char *pStr2 = strstr(pStr1 + 8, "(");
	if (pStr2 == NULL)
	{
		strcpy(p_lpszBuffer, "\0");
		return;
	}
	mid(p_lpszBuffer, pStr1, pStr2 - pStr1 - 9, 9);
}

// 教务系统电子注册
int system_registration()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/index.fcgi\r\n" << GLOBAL_HEADER;
		return -1;
	}

	// 发送请求，获取电子注册信息。
	CCurlTask req;
	if (!req.Exec(false, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>投递电子注册信息失败</b></p><p>curl 操作失败</p>");
		return -1;
	}
	char *m_rep_header = req.GetResult();
	// 查找需要电子注册的学期信息。
	char *pStr1 = strstr(m_rep_header, "selected>");
	if (pStr1 == NULL)
	{
		Error(u8"<p><b>不好意思，电子注册失败。</b></p><p>本学期学费是否已交齐？或请登录教务系统查看具体原因。</p>");
		return -1;
	}
	pStr1 -= 70;
	char *pStr2 = strstr(pStr1, "<option value=\"");
	if (pStr2 == NULL)
	{
		Error(u8"<p>数据错误。不好意思，电子注册失败，请登录教务系统查看具体原因。 (2)</p>");
		return -1;
	}
	pStr1 = pStr2;
	pStr2 = strstr(pStr1 + 16, "\"");
	if (pStr2 == NULL)
	{
		Error(u8"<p>数据错误。不好意思，电子注册失败，请登录教务系统查看具体原因。 (3)</p>");
		return -1;
	}

	char m_regval[4096] = { 0 };
	mid(m_regval, pStr1, pStr2 - pStr1 - 15, 15);

	// 填充电子注册信息
	char m_post_reg_info[4096] = "zxjxjhh=";
	strcat(m_post_reg_info, m_regval);
	int m_post_reg_info_length = strlen(m_post_reg_info);

	// 填充注册请求
	char m_post_req[8192] = { 0 };
	sprintf(m_post_req, REQUEST_POST_REGISTER_INTERFACE, m_regval);

	CCurlTask req2;
	// 开始电子注册
	if (!req2.Exec(false, m_post_req, CGI_HTTP_COOKIE, true, m_post_reg_info))
	{
		Error(u8"<p><b>POST电子注册信息失败</b></p><p>curl 操作失败</p>");
		return -1;
	}
	m_rep_header = req2.GetResult();
	// 检查是否注册成功。
	pStr1 = strstr(m_rep_header, "\xd7\xa2\xb2\xe1\xb3\xc9\xb9\xa6" /*注册成功*/);
	if (pStr1 == NULL)
	{
		Error(u8"<p>不好意思，电子注册失败，请登录教务系统查看具体原因。 (4)</p>");
		return -1;
	}

	//cout << "Status: 302 Found\r\n" << "Location: " << getAppURL().c_str() << "/query.fcgi\r\n" << GLOBAL_HEADER;
	return 0;
}

// 登录学生
bool student_login(char *p_xuehao, char *p_password, char *p_captcha)
{
	// 发送登陆请求。
	const char *m_origin = "zjh1=&tips=&lx=&evalue=&eflag=&fs=&dzslh=&zjh=%s&mm=%s&v_yzm=%s";
	char m_padding[512] = { 0 };
	sprintf(m_padding, m_origin, p_xuehao, p_password, p_captcha);

	CCurlTask req;
	if (!req.Exec(false, REQUEST_LOGIN, CGI_HTTP_COOKIE, true, m_padding))
	{
		Error(u8"<p><b>登录失败</b></p><p>curl 操作失败</p>");
		return false;
	}
	char *m_rep_body = req.GetResult();

	// 拉取登录结果。
	char *m_result = m_rep_body;

	// 处理登录结果。
	char *m_login_not_auth = strstr(m_result, "\xd6\xa4\xbc\xfe\xba\xc5" /*"证件号"*/); // for some urp systems
	if (m_login_not_auth != NULL)
	{
		Error(u8"<p><b>学号或密码不对啊</b></p><p>如果你曾修改过教务系统的帐号密码，请使用新密码再试一试。</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "\xc3\xdc\xc2\xeb\xb2\xbb\xd5\xfd\xc8\xb7" /*密码不正确*/);
	if (m_login_not_auth != NULL)
	{
		Error(u8"<p><b>学号或密码不对啊</b></p><p>如果你曾修改过教务系统的帐号密码，请使用新密码再试一试。</p>");
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
		Error(u8"<p>学院系统君说数据库繁忙</p><p>请先等等再来吧~</p>");
		return false;
	}
	char *m_login_success = strstr(m_result, "\xd1\xa7\xb7\xd6\xd6\xc6\xd7\xdb\xba\xcf\xbd\xcc\xce\xf1" /*学分制综合教务*/);
	if (m_login_success == NULL)
	{
		Error(u8"<p>发生了未知错误</p><p>建议你稍候再试试吧。</p>");
		return false;
	}

	// <del>SQLite3</del> 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
	std::string query("SELECT `id` FROM `UserInfo` WHERE id='");
	query += p_xuehao;
	query += "';";

	MYSQL_RES *result;
	MYSQL_ROW row;
	char id[64] = { 0 };

	if (mysql_query(&db, query.c_str()) != 0)
	{
		student_logout();
		std::string Err_Msg(u8"<b>很抱歉，登录失败。</b><p>数据库错误 (");
		Err_Msg += mysql_error(&db);
		Err_Msg += ")</p>";
		Error(Err_Msg.c_str());
		return false;
	}
	result = mysql_store_result(&db);
	if (mysql_num_rows(result))
	{
		while ((row = mysql_fetch_row(result)))
		{
			if (row[0])
			{
				sprintf(id, "%s", row[0]);
			}
			break;
		}
	}

	mysql_free_result(result);

	// 对密码做URL解码
	int len = url_decode(p_password, strlen(p_password));
	char temp[128] = { 0 };
	left(temp, p_password, len);
	strcpy(p_password, temp);

	if (strlen(id) == 0) // 无记录，则写入数据库
	{
		std::string query("INSERT INTO `UserInfo` (`id`, `password`, `name`, `openid`, `lastlogin`) VALUES ('");
		query += p_xuehao;
		query += "', '";
		query += p_password;
		query += "', '";
		char m_stname[128] = { 0 };
		get_student_name(m_stname);
		query += m_stname;
		query += "', NULL, '";
		char m_time[128] = { 0 };
		get_time(m_time);
		query += m_time;
		query += "');";

		if (mysql_query(&db, query.c_str()) != 0)
		{
			student_logout();
			std::string Err_Msg(u8"<b>很抱歉，登录失败。</b><p>数据库错误 (");
			Err_Msg += mysql_error(&db);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			return false;
		}
	}
	else // 为成功登录的学生更新记录
	{
		std::string query("UPDATE `UserInfo` SET password='");
		char m_time[128] = { 0 };
		get_time(m_time);
		query += p_password;
		query += "', lastlogin='";
		query += m_time;
		query += "', name='";
		char m_stname[128] = { 0 };
		get_student_name(m_stname);
		query += m_stname;
		query += "' WHERE id = '";
		query += p_xuehao;
		query += "';";

		if (mysql_query(&db, query.c_str()) != 0)
		{
			student_logout();
			std::string Err_Msg(u8"<b>很抱歉，登录失败。</b><p>数据库错误 (");
			Err_Msg += mysql_error(&db);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			return false;
		}
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

// 免密查询入口 (/QuickQuery.fcgi)
void parse_QuickQuery_Intro()
{
	bool m_need_update_cookie = false;
	std::string nullphoto;
	process_cookie(&m_need_update_cookie, nullphoto);

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";

	cout << GLOBAL_HEADER;

	if (!isAjaxRequest)
	{
		std::string title(u8"免密成绩查询 - ");
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	cout << strformat( m_lpszQuery.c_str(), APP_NAME, g_users, g_QueryCounter);
	if (!isAjaxRequest)
	{
		cout << footer.c_str();
	}
}

// 免密查询结果 (/query.fcgi?act=QuickQuery)
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
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
		Error(u8"<p>发生错误，POST 数据长度异常</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);	// TORESEARCH
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

	// 获取学号
	char *pStr1 = strstr(m_post_data, "xh=");
	if (pStr1 == NULL)
	{
		free(m_post_data);
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
		Error(u8"<p>无法获取学号信息。</p>");
		return;
	}
	char *pStr2 = strstr(pStr1 + 3, "&");
	char m_xuehao[1024] = { 0 };
	right(m_xuehao, pStr1, strlen(pStr1) - 3);
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
	pStr1 = NULL;
	pStr2 = NULL;
	if (m_xhgs > 5 || m_xhgs <= 0)
	{
		free(m_post_data);
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
		Error(u8"<p>输入的学号个数存在问题，请确认！</p>");
		return;
	}
	
	std::string m_list;
	char m_xxmz[512] = { 0 };
	free(m_post_data);

	for (int xh_index = 0; xh_index < m_xhgs; xh_index++)
		{
			if (strlen(m_xh[xh_index]) > 36)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>输入的学号中有长度存在问题，请确认！</p>");
				return;
			}

			char m_query_param[1024] = { 0 };
			sprintf(m_query_param, "LS_XH=%s", m_xh[xh_index]);
			strcat(m_query_param, "&resultPage=%3F"); // this is ok.

			CCurlTask req;
			if (!req.Exec(true, REQUEST_SET_REPORT_PARAMS, CGI_HTTP_COOKIE, true, m_query_param))
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>投递免密查询请求失败。</p><p>请确认教务系统是可用的。</p>");
				return;
			}

			char *m_lpvBuffer = req.GetResult();
			pStr1 = strstr(m_lpvBuffer, "&reportParamsId=");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>获取报表ID错误。(1)</p>");
				return;
			}
			pStr2 = strstr(pStr1 + 16, "\r\n");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>获取报表ID错误。(2)</p>");
				return;
			}
			char m_paramsID[512] = { 0 };
			mid(m_paramsID, pStr1 + 16, pStr2 - pStr1 - 16, 0);
			pStr1 = NULL;
			pStr2 = NULL;

			char m_query_report[128] = { 0 };
			sprintf(m_query_report, REQUEST_REPORT_FILES, m_paramsID);
			CCurlTask req2;
			std::string referer(REFERER_REQUEST_REPORT_FILES);
			referer.insert(0, SERVER_URL);
			req2.SetReferer(referer);

			if (!req2.Exec(false, m_query_report, CGI_HTTP_COOKIE))
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>通过ID免密查询失败。</p><p>发生了天知道的错误。</p>");
				return;
			}
			m_lpvBuffer = req2.GetResult();
			pStr1 = strstr(m_lpvBuffer, "Exception: ");
			if (pStr1 != NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>教务系统出错了，免密查询失败，请稍后重试~</p>");
				return;
			}
			pStr1 = NULL;
			pStr2 = NULL;

			pStr1 = strstr(m_lpvBuffer, "com.runqian.report.view.text.TextFileServlet");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>免密查询返回参数错误。(1)</p>");
				return;
			}
			pStr2 = strstr(pStr1 + 46, "\";");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>免密查询返回参数错误。(2)</p>");
				return;
			}
			char m_txt_req_path[512] = { 0 };
			mid(m_txt_req_path, pStr1 + 45, pStr2 - pStr1 - 45, 0);

			char m_query_score[512] = { 0 };
			sprintf(m_query_score, REQUEST_TXT_SCORES, m_txt_req_path);
			CCurlTask req3;
			referer.erase();
			referer = strformat(REFERER_REQUEST_TXT_SCORES, m_paramsID);
			referer.insert(0, SERVER_URL);
			req3.SetReferer(referer);

			if (!req3.Exec(false, m_query_score, CGI_HTTP_COOKIE))
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>接收分数信息失败！</p>");
				return;
			}

			m_lpvBuffer = req3.GetResult();
			pStr1 = strstr(m_lpvBuffer, "\xd0\xd5\xc3\xfb\x09" /*姓名\t*/);
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>学生姓名获取失败！(1)</p>");
				return;
			}
			pStr2 = strstr(pStr1 + 4, "\t\t");
			if (pStr2 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>学生姓名获取失败！(2)</p>");
				return;
			}

			if ((pStr2 - pStr1) <= 4)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error, 
					u8"<p><b>出现错误。请确认所输信息是正确的</b></p><p>发生错误的学号: %s</p>", 
					m_xh[xh_index]);
				Error(m_friendly_error);
				return;
			}
			mid(m_xxmz, pStr1 + 4, pStr2 - pStr1 - 5, 1);
			if (strlen(m_xxmz) < 2)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					u8"<p><b>获取信息失败，请确认输入正确。</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				return;
			}
			char m_xxmz_div[128] = { 0 };
			sprintf(m_xxmz_div, "<div class=\"weui-cells__title\">%s</div>", m_xxmz);

			char *m_xxmz_htmlu8 = (char *)malloc(512);
			unsigned int u8len = 0;
			gbk_to_utf8(m_xxmz_div, (unsigned int)strlen(m_xxmz_div), &m_xxmz_htmlu8, &u8len);
			std::string m_xxmz_html(m_xxmz_htmlu8);
			free(m_xxmz_htmlu8);

			pStr1 = NULL;
			pStr2 = NULL;
			pStr1 = strstr(m_lpvBuffer, "\xbf\xbc\xca\xd4\xca\xb1\xbc\xe4\x09\x09\x0a" /*"考试时间\t\t\n"*/);
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					u8"<p><b>接收到的报表存在问题。</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				return;
			}

			if (strlen(m_lpvBuffer) <= 800)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					u8"<p><b>收到的报表大小存在问题</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
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
					strcpy(m_cj, m_red);
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
					strcpy(m_kcmz, m_kcmz_cx);
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

				strcpy(m_test_info[m_index].cj, m_cj);
				strcpy(m_test_info[m_index].kcmz, m_kcmz);
				m_test_info[m_index].date = atoi(m_date_4);
				m_index++;
			}

			if (!m_success)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error(u8"<p>抱歉，免密查询过程中失败，请稍后再试</p>");
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
						char m_temp[1024] = { 0 };
						sprintf(m_temp, QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
						char *m_u8tmp = (char *)malloc(4096);
						unsigned int u8len = 0;
						gbk_to_utf8(m_temp, (unsigned int)strlen(m_temp), &m_u8tmp, &u8len);
						m_list.append(m_u8tmp);
						free(m_u8tmp);
					}
				}
				else if (m_test_info[i].date == m_max_date || m_test_info[i].date == m_secondary_max)
				{
					char m_temp[1024] = { 0 };
					sprintf(m_temp, QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
					char *m_u8tmp = (char *)malloc(4096);
					unsigned int u8len = 0;
					gbk_to_utf8(m_temp, (unsigned int)strlen(m_temp), &m_u8tmp, &u8len);
					m_list.append(m_u8tmp);
					free(m_u8tmp);
				}
			}
			m_list.append("</div>");
		}

		cout << GLOBAL_HEADER;

		if (m_xhgs > 1)
		{
			if (!isAjaxRequest)
			{
				std::string title(u8"多人查询 - 免密成绩查询 - ");
				title += APP_NAME;
				cout << strformat(header.c_str(), title.c_str());
			}
			cout << strformat( m_lpszQuery.c_str(), u8"多人查询", m_list.c_str());
		}
		else
		{
			char *m_xxmz_u8 = (char *)malloc(512);
			unsigned int u8len = 0;
			gbk_to_utf8(m_xxmz, (unsigned int)strlen(m_xxmz), &m_xxmz_u8, &u8len);

			if (!isAjaxRequest)
			{
				std::string title(m_xxmz_u8);
				title += u8" - 免密成绩查询 - ";
				title += APP_NAME;
				cout << strformat(header.c_str(), title.c_str());
			}
			cout << strformat( m_lpszQuery.c_str(), m_xxmz_u8, m_list.c_str());
			free(m_xxmz_u8);
		}
		if (!isAjaxRequest)
		{
			cout << footer.c_str();
		}

		g_QueryCounter += m_xhgs;
		SetQueryCounter(g_QueryCounter);
}

// QQ帐号绑定入口与解绑逻辑 (/OAuth2Assoc.fcgi)
void OAuth2_Association(bool isPOST)
{
	if (CGI_QUERY_STRING == NULL)
	{
		Error(u8"参数错误 (Null QUERY_STRING)");
		return;
	}

	// 解除绑定逻辑
	char *pStr1 = strstr(CGI_QUERY_STRING, "release=");
	if (pStr1 != NULL)
	{
		char student_id[512] = { 0 };
		get_student_id(student_id);
		if (student_id == NULL)
		{
			Error(u8"非法操作！ (尚未登录)");
			return;
		}
		char *pStr2 = strstr(pStr1 + 8, "&");
		char *releaseid = new char[strlen(CGI_QUERY_STRING)];
		if (pStr2 == NULL)
		{
			right(releaseid, pStr1 + 8, strlen(CGI_QUERY_STRING) - 8);
		}
		else
		{
			mid(releaseid, pStr1 + 8, pStr2 - pStr1 - 8, 0);
		}
		if (strcmp(releaseid, student_id) != 0)
		{
			Error(u8"非法操作！ (身份错误)");
			return;
		}

		std::string query("UPDATE `UserInfo` SET `openid`=NULL, `OAuth_name`=NULL, `OAuth_avatar`=NULL WHERE id='");
		query += student_id;
		query += "';";

		if (mysql_query(&db, query.c_str()) != 0)
		{
			std::string Err_Msg(u8"<b>解除绑定失败，请稍后再试。</b><p>(");
			Err_Msg += mysql_error(&db);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			return;
		}

		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/main.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	pStr1 = strstr(CGI_QUERY_STRING, "openid=");
	if (pStr1 == NULL)
	{
		Error(u8"鉴权失败 (Null openid)");
		return;
	}
	char *pStr2 = strstr(pStr1 + 7, "&");
	char *openid = new char[strlen(CGI_QUERY_STRING)];
	if (pStr2 == NULL)
	{
		right(openid, pStr1 + 7, strlen(CGI_QUERY_STRING) - 7);
	}
	else
	{
		mid(openid, pStr1 + 7, pStr2 - pStr1 - 7, 0);
	}
	if (!isPOST)
	{
		// 如果传进 sid，则自动填写学号、并且从数据库中拿密码。
		pStr1 = strstr(CGI_QUERY_STRING, "stid=");
		char stid[128] = { 0 };
		if (pStr1 != NULL)
		{
			pStr2 = strstr(pStr1 + 5, "&");
			if (pStr2 == NULL)
			{
				right(stid, pStr1 + 5, strlen(CGI_QUERY_STRING) - 5);
			}
			else
			{
				mid(stid, pStr1 + 5, pStr2 - pStr1 - 5, 0);
			}
			DeCodeStr(stid);
		}

		char pass[512] = {0};
		if (strlen(stid) != 0 && strcmp(stid, "NONE") != 0)
		{
			// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
			std::string query("SELECT `password` FROM `UserInfo` WHERE id='");
			query += stid;
			query += "';";

			MYSQL_RES *result;
			MYSQL_ROW row;
			if (mysql_query(&db, query.c_str()) != 0)
			{
				char Err_Msg[1024] = u8"<b>数据库准备失败！请确认数据库合法性。</b><p>(";
				strcat(Err_Msg, mysql_error(&db));
				strcat(Err_Msg, ")</p>");
				Error(Err_Msg);
				delete[]openid;
				return;
			}
			result = mysql_store_result(&db);
			if (mysql_num_rows(result))
			{
				while ((row = mysql_fetch_row(result)))
				{
					if (row[0])
					{
						sprintf(pass, "%s", row[0]);
					}
					break;
				}
			}
			mysql_free_result(result);
		}

		std::string m_lpszHomepage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

		cout << GLOBAL_HEADER;

		std::string title(u8"QQ用户绑定 - ");
		title += APP_NAME;
		cout << strformat( header.c_str(), title.c_str());

		if (strlen(stid) == 0 || strcmp(stid, "NONE") == 0)
		{
			cout << strformat( m_lpszHomepage.c_str(), APP_NAME, openid, u8"感谢使用QQ登录，请先绑定自己的学号吧 :)",
				 "", pass);
		}
		else if(strlen(pass) == 0)
		{
			cout << strformat( m_lpszHomepage.c_str(), APP_NAME, openid, u8"感谢使用QQ登录，请输入密码来继续操作 :)",
				stid, "");
		}
		else
		{
			cout << strformat( m_lpszHomepage.c_str(), APP_NAME, openid, u8"感谢使用QQ登录，请输入验证码来继续操作 :)",
				stid, pass);
		}
		cout << footer.c_str();
	}
	else // 提交帐号密码验证码，打算登录绑定了
	{
		// 获取 POST 数据。
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0)
		{
			Error(u8"<p>发生错误，POST 数据长度异常</p>");
			delete[]openid;
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

		// 获取学号
		char *pStr1 = strstr(m_post_data, "xh=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error(u8"<p>无法获取学号信息。</p>");
			delete[]openid;
			return;
		}
		char *pStr2 = strstr(pStr1 + 3, "&");
		char m_xuehao[128] = { 0 };
		mid(m_xuehao, pStr1, pStr2 - pStr1 - 3, 3);
		pStr1 = NULL;
		pStr2 = NULL;

		// 获取密码
		pStr1 = strstr(m_post_data, "mm=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error(u8"<p>无法获取密码信息。</p>");
			delete[]openid;
			return;
		}
		pStr2 = strstr(pStr1 + 3, "&");
		char m_password[128] = { 0 };
		mid(m_password, pStr1, pStr2 - pStr1 - 3, 3);
		pStr1 = NULL;
		pStr2 = NULL;

		// 获取验证码
		pStr1 = strstr(m_post_data, "yzm=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error(u8"<p>无法获取验证码信息。</p>");
			delete[]openid;
			return;
		}
		char m_captcha[128] = { 0 };
		right(m_captcha, pStr1 + 4, 4);

		if (!student_login(m_xuehao, m_password, m_captcha))
		{
			free(m_post_data);
			delete[]openid;
			return;
		}

		// 这里表示登录成功，应该写入数据库了。
		std::string query("UPDATE `UserInfo` SET openid='");
		query += openid;
		query += "' WHERE id='";
		query += m_xuehao;
		query += "';";

		if (mysql_query(&db, query.c_str()) != 0)
		{
			char Err_Msg[1024] = u8"<b>很抱歉，QQ绑定失败。</b><p>数据库错误 (";
			strcat(Err_Msg, mysql_error(&db));
			strcat(Err_Msg, u8")</p><p>但是还可以正常登录的。</p>");
			Error(Err_Msg);
			free(m_post_data);
			delete[]openid;
			return;
		}

		cout << "Status: 302 Found\r\n";
		cout << "Location: " << getAppURL().c_str() << "/main.fcgi\r\n";
		cout << GLOBAL_HEADER;
		
		free(m_post_data);
	}

	delete[]openid;
	return;
}

// 教学评估页面 (/TeachEval.fcgi)
void parse_teaching_evaluation()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // 还没登陆就丢去登陆。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/index.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	// 检查是否需要教学评估
	CCurlTask req;
	if (!req.Exec(false, GET_TEACH_EVAL_LIST, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>教学评估请求投递失败</b></p><p>curl 操作失败</p>");
		return;
	}

	char *m_rep_body = req.GetResult();
	char *m_result = strstr(m_rep_body, "\xd1\xa7\xc9\xfa\xc6\xc0\xb9\xc0\xce\xca\xbe\xed\xc1\xd0\xb1\xed" /*"学生评估问卷列表"*/);
	if (m_result == NULL)
	{
		Error(u8"<p>从服务器拉取教学评估信息失败。</p>");
		return;
	}

	m_result = strstr(m_rep_body, "\xb7\xc7\xbd\xcc\xd1\xa7\xc6\xc0\xb9\xc0\xca\xb1\xc6\xda\xa3\xac\xbb\xf2\xc6\xc0\xb9\xc0\xca\xb1\xbc\xe4\xd2\xd1\xb9\xfd" /*非教学评估时期，或评估时间已过*/);
	if (m_result != NULL)
	{
		Error(u8"<p>学院还没有开放评教呢，或者你来晚了哦</p>");
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
		char dst[10][128] = {0};

		int split_ret = split(dst, img_txt, "#@");

		if (split_ret != 6)
		{
			Error(u8"<p>评教条目数目分割失败</p>");
			return;
		}

		strcpy(te[counts].wjbm, dst[0]);
		strcpy(te[counts].bpr, dst[1]);
		strcpy(te[counts].pgnr, dst[5]);
		strcpy(te[counts].name, dst[4]);

		counts++;
		m_result1 = strstr(m_result, "\x3C\x74\x64\x20\x61\x6C\x69\x67\x6E\x3D\x22\x63\x65\x6E\x74\x65\x72\x22\x3E\xCA\xC7\x3C\x2F\x74\x64\x3E" /*"<td align=\"center\">是</td>"*/);
		m_result = strstr(m_result + 11, "<img name=\"");
	}

	int to_eval = 0;
	std::string to_eval_list = "<div class=\"weui-cells\">";
	for (int i = 0; i < counts; i++)
	{
			to_eval_list += "<div class=\"weui-cell\"><div class=\"weui-cell__bd\"><p>";
			to_eval_list += te[i].name;
			to_eval_list += "</p></div><div class=\"weui-cell__ft\">";
			if (te[i].evaled == false)
			{
				to_eval_list += u8"未评价";
				to_eval++;
			}
			else
			{
				to_eval_list += u8"<b style=\"color:#00a70e\">已评价</b>";
			}
			to_eval_list += "</div></div>";
	}
	to_eval_list += "</div>";

	std::string m_lpszTeachEvalPage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	std::string outer;
	char out_head[1024] = { 0 };

	cout << GLOBAL_HEADER;

	if (!isAjaxRequest)
	{
		std::string title = u8"一键评教 - ";
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}

	bool need_eval = true;
	if (to_eval && counts)
	{
		sprintf(out_head, 
			u8"<div class=\"weui-cells__title\">嗯，当前还有 %d 门课程需要评估，总共 %d 门。</div>", 
			to_eval, counts);
	}
	else
	{
		strcpy(out_head, u8"<div class=\"weui-cells__title\"><p>嗯，你都评价好啦。真是好宝宝 O(∩_∩)O</div>");
		need_eval = false;
	}

	outer.append(out_head);
	outer.append(to_eval_list);

	cout << strformat(
		m_lpszTeachEvalPage.c_str(),
		need_eval ? u8"老师很辛苦，给个赞呗。默认全好评，你懂的 :)" : "",
		need_eval ? "block" : "none"
		, outer.c_str());
	if (!isAjaxRequest)
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

	if (m_photo.empty()) // 还没登陆就丢去登陆。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/index.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		Error(u8"<p>发生错误，POST 数据长度异常</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

	// 获取主观评价
	char *pStr1 = strstr(m_post_data, "nr=");
	if (pStr1 == NULL)
	{
		free(m_post_data);
		Error(u8"<p>无法获取主观评价内容</p>");
		return;
	}

	char zgpj[1024] = { 0 };
	left(zgpj, pStr1 + 3, m_post_length - 3);
	free(m_post_data);

	// 检查是否需要教学评估
	CCurlTask req;
	if (!req.Exec(false, GET_TEACH_EVAL_LIST, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>教学评估请求投递失败</b></p><p>curl 操作失败</p>");
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
		char dst[10][128] = { 0 };

		int split_ret = split(dst, img_txt, "#@");

		if (split_ret != 6)
		{
			Error(u8"<p>评教条目数目分割失败</p>");
			return;
		}

		strcpy(te[counts].wjbm, dst[0]);
		strcpy(te[counts].bpr, dst[1]);
		strcpy(te[counts].pgnr, dst[5]);
		strcpy(te[counts].name, dst[4]);
		int new_len;
		char *tmp = url_encode(dst[3], strlen(dst[3]), &new_len);
		left(te[counts].wjmc, tmp, new_len);
		tmp = url_encode(dst[2], strlen(dst[2]), &new_len);
		left(te[counts].bprm, tmp, new_len);
		tmp = url_encode(dst[4], strlen(dst[4]), &new_len);
		left(te[counts].pgnrm, tmp, new_len);

		counts++;
		m_result1 = strstr(m_result, "\x3C\x74\x64\x20\x61\x6C\x69\x67\x6E\x3D\x22\x63\x65\x6E\x74\x65\x72\x22\x3E\xCA\xC7\x3C\x2F\x74\x64\x3E" /*"<td align=\"center\">是</td>"*/);
		m_result = strstr(m_result + 11, "<img name=\"");

		free(tmp);
	}

	int to_eval = 0;
	for (int i = 0; i < counts; i++)
	{
		if (te[i].evaled == false)
			to_eval++;
	}

	std::string outer;
	char out_head[1024] = { 0 };
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
					Error(u8"<p><b>准备评估时发生了错误</b></p><p>curl 操作失败</p>");
					return;
				}

				m_rep_body = req2.GetResult();
				char *m_result = strstr(m_rep_body, "\xce\xca\xbe\xed\xc6\xc0\xb9\xc0\xd2\xb3\xc3\xe6" /*"问卷评估页面"*/);
				if (m_result == NULL)
				{
					std::string err_msg = "<p>出现错误</p><p>很抱歉，在评估《";
					err_msg = err_msg + te[i].name + "》课程时出现了错误。</p><p>(进入详细页面失败)</p>";
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
						std::string err_msg = "<p>出现错误</p><p>很抱歉，在评估《";
						err_msg = err_msg + te[i].name + "》课程时出现了错误。</p><p>(名称条目引号闭合失败)</p>";
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
						strcpy(last, num);
					}
					char *p2 = strstr(p1 + 1, "value=\"");
					if (p2 == NULL)
					{
						std::string err_msg = "<p>出现错误</p><p>很抱歉，在评估《";
						err_msg = err_msg + te[i].name + "》课程时出现了错误。</p><p>(值条目引号开启失败)</p>";
						Error(err_msg.c_str());
						return;
					}
					char *p3 = strstr(p2 + 7, "\"");
					if (p2 == NULL)
					{
						std::string err_msg = "<p>出现错误</p><p>很抱歉，在评估《";
						err_msg = err_msg + te[i].name + "》课程时出现了错误。</p><p>(值条目引号闭合失败)</p>";
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
					std::string err_msg = "<p>出现错误</p><p>很抱歉，在评估《";
					err_msg = err_msg + te[i].name + "》课程时出现了错误。</p><p>curl 操作失败</p>";
					Error(err_msg.c_str());
				}

				m_rep_body = req3.GetResult();
				m_result = strstr(m_rep_body, "\xb3\xc9\xb9\xa6" /*"成功"*/);
				if (m_result == NULL)
				{
					std::string err_msg = "<p>出现错误</p><p>很抱歉，在评估《";
					err_msg = err_msg + te[i].name + "》课程时出现了错误。</p>";
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

	if (m_photo.empty()) // 还没登陆就丢去登陆。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/index.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
	cout << GLOBAL_HEADER;

	if (!isAjaxRequest)
	{
		std::string title(u8"修改密码 - ");
		title += APP_NAME;
		cout << strformat(header.c_str(), title.c_str());
	}
	cout << strformat( m_lpszQuery.c_str());

	if (!isAjaxRequest)
	{
		cout << footer.c_str();
	}
}

// 修改密码 (POST /changePassword.fcgi)
void do_change_password() //(POST /changePassword.fcgi)
{
	// modifyPassWordAction.do?pwd=
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // 有数据，需要获取照片
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // 还没登陆就丢去登陆。
	{
		cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/index.fcgi\r\n" << GLOBAL_HEADER;
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

	// 获取新密码
	char *pStr1 = strstr(m_post_data, "mm=");
	if (pStr1 == NULL)
	{
		free(m_post_data);
		Error(u8"<p>发生了错误，无法获取 POST 数据。</p>");
		return;
	}

	char pwd[128] = { 0 };
	left(pwd, pStr1 + 3, m_post_length - 3);
	int len = url_decode(pwd, strlen(pwd));
	char temp[128];
	left(temp, pwd, len);
	strcpy(pwd, temp);
	free(m_post_data);

	if (len > 12 || len <= 0)
	{
		Error(u8"<p>新密码长度不能超过12个字符！</p>");
		return;
	}

	char GET_RET[1024] = { 0 };
	sprintf(GET_RET, REQ_CHANGE_PASSWORD, pwd);

	CCurlTask req;
	if (!req.Exec(false, GET_RET, CGI_HTTP_COOKIE))
	{
		Error(u8"<p><b>修改密码时发生了错误</b></p><p>curl 操作失败</p>");
		return;
	}

	// 拉取改密结果
	char *m_rep_header = req.GetResult();
	pStr1 = strstr(m_rep_header, "\xb3\xc9\xb9\xa6" /*"成功"*/);
	if (pStr1 == NULL)
	{
		Error(u8"<p>密码修改失败，请确认是否输入了非法字符，或请稍后再试。</p>");
		return;
	}

	// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128) text lastlogin(64)。
	std::string query("UPDATE `UserInfo` SET password='");
	char m_time[128] = { 0 };
	get_time(m_time);
	query += pwd;
	query += "', lastlogin='";
	query += m_time;
	query += "' WHERE id = '";
	char id[128] = { 0 };
	get_student_id(id);
	query += id;
	query += "';";

	if (mysql_query(&db, query.c_str()) != 0)
	{
		std::string Err_Msg(u8"<b>密码修改成功，但登录数据库记录失败，请稍后再试。(请使用新密码登录)</b><p>(");
		Err_Msg += mysql_error(&db);
		Err_Msg += ")</p>";
		Error(Err_Msg.c_str());
		return;
	}

	student_logout();
	cout << "Status: 302 Found\r\nLocation: " << getAppURL().c_str() << "/index.fcgi\r\n" << GLOBAL_HEADER;
}