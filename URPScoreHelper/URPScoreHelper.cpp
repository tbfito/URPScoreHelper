// URPScoreHelper.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "General.h"
#include "StringHelper.h"
#include "CrawlFactory.h"
#include "URPScoreHelper.h"
#include "OAuth2.h"
#include "resource.h"

using namespace std;

// 入口函数
int main()
{
	LoadConfig();
	g_start_time = GetTickCount();

	// 初始化 Socket 库。
	if (!InitSocketLibrary())
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>Socket 初始化失败！</p>";
		return -1;
	}

		CGI_REQUEST_METHOD = getenv("REQUEST_METHOD"); // 请求方法
		CGI_CONTENT_LENGTH = getenv("CONTENT_LENGTH"); // 数据长度
		CGI_SCRIPT_NAME = getenv("SCRIPT_NAME"); // 脚本名称
		CGI_QUERY_STRING = getenv("QUERY_STRING"); // 查询参数
		CGI_PATH_TRANSLATED = getenv("PATH_TRANSLATED"); // 脚本位置
		CGI_HTTP_COOKIE = getenv("HTTP_COOKIE"); // Cookie
		CGI_SERVER_SOFTWARE = getenv("SERVER_SOFTWARE"); // 服务器软件

		// 获取多少用户使用了我们的服务 :)
		g_fQueryCount = fopen("QueryCount.bin", "r+");
		g_QueryCount = 0;
		if (g_fQueryCount != NULL)
		{
			fscanf(g_fQueryCount, "%ld", &g_QueryCount);
		}
		else
		{
			g_fQueryCount = fopen("QueryCount.bin", "w+");
		}
		if (g_fQueryCount == NULL)
		{
			cout << "Status: 500 Internal Server Error\n"
				<< GLOBAL_HEADER
				<< "<p>fopen() 失败！</p>";
			return 0;
		}
		fseek(g_fQueryCount, 0, SEEK_SET);

		if (CGI_REQUEST_METHOD == NULL || CGI_CONTENT_LENGTH == NULL || CGI_SCRIPT_NAME == NULL || CGI_QUERY_STRING == NULL ||
			CGI_PATH_TRANSLATED == NULL || CGI_HTTP_COOKIE == NULL)
		{
			cout << "Status: 500 Internal Server Error\n"
				<< GLOBAL_HEADER
				<< "<p>CGI 接口异常，请检查设置。</p>";
			return 0;
		}

		if (!LoadPageSrc())
		{
			return -1;
		}

		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0) // 如果是 GET 请求
		{
			if (strcmp(CGI_SCRIPT_NAME, "/") == 0 || strcmp(CGI_SCRIPT_NAME, "/index.cgi") == 0 || strcmp(CGI_SCRIPT_NAME, "/main.cgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=logout") == 0)
				{
					student_logout();
					cout << "Status: 302 Found\n" << "Location: index.cgi\n" << GLOBAL_HEADER;
					return 0;
				}
				if (strcmp(CGI_QUERY_STRING, "act=requestAssoc") == 0)
				{
					bool m_need_update_cookie = false;
					process_cookie(&m_need_update_cookie, NULL);
					char student_id[36] = { 0 };
					get_student_id(student_id);
					student_logout();
					cout << "Status: 302 Found\n" << "Location: OAuth2.cgi?stid="<< student_id << "\n" << GLOBAL_HEADER;
					return 0;
				}
				parse_index();
				return 0;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2.cgi") == 0)
			{
				OAuth2_process();
				return 0;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2CallBack.cgi") == 0)
			{
				OAuth2_CallBack();
				return 0;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2Assoc.cgi") == 0)
			{
				OAuth2_Association(false);
				return 0;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/query.cgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=system_registration") == 0)
				{
					system_registration();
					ZeroMemory(JSESSIONID, 256);
					return -1;
				}
				parse_query();
				return 0;
			}

			if (strcmp(CGI_SCRIPT_NAME, "/QuickQuery.cgi") == 0)
			{
				parse_QuickQuery_Intro();
				return 0;
			}

			cout << "Status: 404 No Such CGI Page\n";
			Error("<p>找不到该页面。</p>");
			
			
			ZeroMemory(JSESSIONID, 256);
			return -1;
		}

		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0) // 如果是 POST 请求
		{
			if (strcmp(CGI_SCRIPT_NAME, "/query.cgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=QuickQuery") == 0)
				{
					parse_QuickQuery_Result();
					return 0;
				}
				parse_query();
				return 0;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/main.cgi") == 0)
			{
				parse_main(false, NULL, true);
				return 0;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/OAuth2Assoc.cgi") == 0)
			{
				OAuth2_Association(true);
				return 0;
			}
		}

	puts("Status: 500 Internal Server Error");
	Error("<p>发生错误，未经处理的异常。</p>");
	
	return -1;
}

// 预加载头部和尾部页面(header.cgi, footer.cgi, error.cgi)
bool LoadPageSrc()
{
	// 读入主页面文件
	char *pStr = strstr((char *)CGI_PATH_TRANSLATED, "\\");
	bool isUnixBasedPath = (pStr == NULL);
	if (isUnixBasedPath)
	{
		pStr = strstr((char *)CGI_PATH_TRANSLATED, "/");
	}
	if (pStr == NULL)
	{
		return false;
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

	mid(doc_root, (char *)CGI_PATH_TRANSLATED, Last - (char *)CGI_PATH_TRANSLATED + 1, 0);
	char *file_root = new char[MAX_PATH];
	memset(file_root, 0, MAX_PATH);

	strcpy(file_root, doc_root);
	strcat(file_root, "header.cgi");
	FILE *m_file = fopen(file_root, "rb");
	if (m_file == NULL)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>找不到 header.cgi 模板内容。</p>";
		delete[]doc_root;
		delete[]file_root;
		return false;
	}
	fseek(m_file, 0, SEEK_END); // 移到尾
	int m_file_length = ftell(m_file);
	fseek(m_file, 0, SEEK_SET);
	char *m_lpszHomepage = (char *)malloc(m_file_length + 1);
	ZeroMemory(m_lpszHomepage, m_file_length + 1);
	if (fread(m_lpszHomepage, m_file_length, 1, m_file) != 1)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>无法读取 header.cgi 模板内容。</p>";
		fclose(m_file);
		free(m_lpszHomepage);
		delete[]doc_root;
		delete[]file_root;
		return false;
	}
	fclose(m_file);
	header = m_lpszHomepage;

	strcpy(file_root, doc_root);
	strcat(file_root, "footer.cgi");
	m_file = fopen(file_root, "rb");
	if (m_file == NULL)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>找不到 footer.cgi 模板内容。</p>";
		delete[]doc_root;
		delete[]file_root;
		return false;
	}
	fseek(m_file, 0, SEEK_END); // 移到尾
	m_file_length = ftell(m_file);
	fseek(m_file, 0, SEEK_SET);
	m_lpszHomepage = (char *)malloc(m_file_length + 1);
	ZeroMemory(m_lpszHomepage, m_file_length + 1);
	if (fread(m_lpszHomepage, m_file_length, 1, m_file) != 1)
	{
		cout << "Status: 500 Internal Server Error\n"
			 << GLOBAL_HEADER
			 << "<p>无法读取 footer.cgi 模板内容。</p>";
		fclose(m_file);
		free(m_lpszHomepage);
		delete[]doc_root;
		delete[]file_root;
		return false;
	}
	fclose(m_file);
	footer = m_lpszHomepage;

	strcpy(file_root, doc_root);
	strcat(file_root, "error.cgi");
	m_file = fopen(file_root, "rb");
	if (m_file == NULL)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>找不到 error.cgi 模板内容。</p>";
		delete[]doc_root;
		delete[]file_root;
		return false;
	}
	fseek(m_file, 0, SEEK_END); // 移到尾
	m_file_length = ftell(m_file);
	fseek(m_file, 0, SEEK_SET);
	m_lpszHomepage = (char *)malloc(m_file_length + 1);
	ZeroMemory(m_lpszHomepage, m_file_length + 1);
	if (fread(m_lpszHomepage, m_file_length, 1, m_file) != 1)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>无法读取 error.cgi 模板内容。</p>";
		fclose(m_file);
		free(m_lpszHomepage);
		delete[]doc_root;
		delete[]file_root;
		return false;
	}
	fclose(m_file);
	error = m_lpszHomepage;

	ERROR_HTML = (char *)malloc(strlen(error) + strlen("出错啦 - ") + strlen(SOFTWARE_NAME) + strlen(footer) + m_file_length + 1);
	char title[256] = "出错啦 - ";
	strcat(title, SOFTWARE_NAME);
	sprintf(ERROR_HTML, header, title);
	strcat(ERROR_HTML, error);
	strcat(ERROR_HTML, footer);

	free(error); // 只有 error 已经处理至 ERROR_HTML 可以释放，其余 header footer 需要常驻内存。
	delete[]doc_root;
	delete[]file_root;
	return true;
}

// 加载配置
void LoadConfig()
{
	SERVER = (char *)malloc(256);
	OAUTH2_APPID = (char *)malloc(1024);
	OAUTH2_SECRET = (char *)malloc(1024);
	memset(SERVER, 0, 256);
	memset(OAUTH2_APPID, 0, 1024);
	memset(OAUTH2_SECRET, 0, 1024);
	char *Dir = (char *)malloc(260);
	GetCurrentDirectoryA(260, Dir);
	strcat(Dir, "\\config.ini");
	GetPrivateProfileStringA("Config", "server", "127.0.0.1", SERVER, 256, Dir);
	GetPrivateProfileStringA("Config", "OAUTH2_APPID", "NULL", OAUTH2_APPID, 1024, Dir);
	GetPrivateProfileStringA("Config", "OAUTH2_SECRET", "NULL", OAUTH2_SECRET, 1024, Dir);
	free(Dir);
}

// 处理 Cookie、照片
int process_cookie(bool *p_need_update_cookie, char *p_photo_uri)
{
	int m_iResult = 0;

	if (strcmp(CGI_HTTP_COOKIE, "") != 0) // 如果客户端已经拿到 JSESSIONID，看看原 Cookie 是否过期、有效（即服务器是否设置了新 Cookie）
	{
		// 申请内存，并接受服务端返回数据。
		char * m_rep_header = (char *)malloc(1024);
		ZeroMemory(m_rep_header, 1024);
		char m_req_homepage_cookie[2048] = { 0 };
		sprintf(m_req_homepage_cookie, REQUEST_HOME_PAGE_WITH_COOKIE, CGI_HTTP_COOKIE);
		if (!CrawlRequest(m_req_homepage_cookie, m_rep_header, 1024, &m_iResult))
		{
			free(m_rep_header);
			return -1;
		}

		char *pStr1 = strstr(m_rep_header, "JSESSIONID=");
		if (pStr1 != NULL)
		{
			char *pStr2 = strstr(pStr1 + 11, ";");
			if (pStr2 == NULL)
			{
				free(m_rep_header);
				cout << "Status: 500 Internal Server Error\n";
				Error("<p>无法获取 Servlet Session ID。</p><p>Cookie 结尾失败。</p>");
				return -1;
			}
			mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // 成功获得新 Session ID。
			*p_need_update_cookie = true;
			free(m_rep_header);
			return 0;
		}
		else // 如果 Cookie 还能用，就获取它。
		{
			char *pStr1 = strstr((char *)CGI_HTTP_COOKIE, "JSESSIONID=");
			if (pStr1 != NULL)
			{
				char *pStr2 = strstr(pStr1 + 11, ";");
				if (pStr2 == NULL) // 如果这条 Cookie 在最后一条
				{
					right(JSESSIONID, (char *)CGI_HTTP_COOKIE, strlen(CGI_HTTP_COOKIE) - (pStr1 - CGI_HTTP_COOKIE) - 11);
				}
				else
				{
					mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11);
				}
			}
		}
		free(m_rep_header);
	}
	else
	{
		// 申请内存，并接受服务端返回数据。
		char * m_rep_header = (char *)malloc(1024);
		ZeroMemory(m_rep_header, 1024);
		if (!CrawlRequest(REQUEST_HOME_PAGE, m_rep_header, 1024, &m_iResult))
		{
			free(m_rep_header);
			return -1;
		}

		// 获取 Session ID。
		char *pStr1 = strstr(m_rep_header, "JSESSIONID=");
		if (pStr1 == NULL)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取 Servlet Session ID。</p><p>Cookie 标头失败。</p>");
			return -1;
		}
		char *pStr2 = strstr(pStr1 + 11, ";");
		if (pStr2 == NULL)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取 Servlet Session ID。</p><p>Cookie 结尾失败。</p>");
			return -1;
		}
		mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // 成功获得 Session ID。

		free(m_rep_header);
		*p_need_update_cookie = true;
	}

	if (p_photo_uri == NULL) return 0; // p_photo_uri 指定了 NULL 代表不需要照片。

	// 看看登录没
	char *m_photo = (char *)malloc(40960);
	ZeroMemory(m_photo, 40960);
	char REQ_PHOTO[1024] = { 0 };
	char Jsess[512] = "JSESSIONID=";
	strcat(Jsess, JSESSIONID);
	sprintf(REQ_PHOTO, REQUEST_PHOTO, Jsess);

	if (!CrawlRequest(REQ_PHOTO, m_photo, 40960, &m_iResult))
	{
		free(m_photo);
		return -1;
	}

	if (strstr(m_photo, "登录") == NULL)
	{
		char *pStr1 = strstr(m_photo, "\r\n\r\n");
		if (pStr1 == NULL)
		{
			free(m_photo);
			return -1;
		}
		pStr1 += 4;
		int m_photoLength = m_iResult - (pStr1 - m_photo);

		char *m_base64 = (char *)malloc(m_photoLength * 3 + 1);
		ZeroMemory(m_base64, m_photoLength * 3 + 1);
		base64_encode((const unsigned char *)pStr1, m_base64, m_photoLength);
		char *m_PhotoDataURI = (char *)malloc(m_photoLength * 3 + 1 + 24);
		ZeroMemory(m_PhotoDataURI, m_photoLength * 3 + 1 + 24);
		strcpy(m_PhotoDataURI, "data:image/jpg;base64,");
		strcat(m_PhotoDataURI, m_base64);
		strcpy(p_photo_uri, m_PhotoDataURI);
		free(m_photo);
		free(m_base64);
		free(m_PhotoDataURI);
	}
	else
	{
		free(m_photo);
	}
	return 0;
}

// 处理 GET /main.cgi
int parse_main(bool p_need_set_cookie, char *p_photo, bool p_is_login)
{
	if (strcmp(CGI_REQUEST_METHOD, "POST") == 0 && p_is_login == true)
	{
		// 获取 POST 数据。
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>发生错误，POST 数据长度异常。</p>");
			return -1;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		if (fgets(m_post_data, m_post_length + 1, stdin) == NULL)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>发生错误，POST 数据拉取失败。</p>");
			return -1;
		}

		// 获取学号
		char *pStr1 = strstr(m_post_data, "xh=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取学号信息。</p>");
			return -1;
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
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取密码信息。</p>");
			return -1;
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
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取验证码信息。</p>");
			return -1;
		}
		char m_captcha[128] = { 0 };
		right(m_captcha, pStr1 + 4, 4);

		if (!student_login(m_xuehao, m_password, m_captcha))
		{
			// 其余资源清理已在学生登录里面做过了。
			free(m_post_data);
			return -1;
		}
		free(m_post_data);
	}
	char *m_photo = p_photo;
	if (m_photo == NULL)
	{
		m_photo = (char *)malloc(102424);
		ZeroMemory(m_photo, 102424);
		process_cookie(&p_need_set_cookie, m_photo);
	}
	// 读入主页面文件
	FILE *m_file_homepage = fopen(CGI_PATH_TRANSLATED, "rb");
	if (m_file_homepage == NULL)
	{
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>错误：找不到主页面模板。</p>");
		return -1;
	}
	fseek(m_file_homepage, 0, SEEK_END); // 移到尾
	int m_file_homepage_length = ftell(m_file_homepage); // 获取文件长度
	fseek(m_file_homepage, 0, SEEK_SET); // 重新移回来
	char *m_lpszHomepage = (char *)malloc(m_file_homepage_length + 1);
	ZeroMemory(m_lpszHomepage, m_file_homepage_length + 1);
	if (fread(m_lpszHomepage, m_file_homepage_length, 1, m_file_homepage) != 1) // 将硬盘数据拷至内存
	{
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>无法读取主页模板内容。</p>");
		fclose(m_file_homepage);
		free(m_lpszHomepage);
		return -1;
	}
	fclose(m_file_homepage); // 关闭文件
	char m_student_name[16] = {0};
	char m_student_id[16] = { 0 };
	get_student_name(m_student_name);
	get_student_id(m_student_id);

	// 输出网页
	if (p_need_set_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";

	cout << GLOBAL_HEADER;

	char title[256] = { 0 };
	strcpy(title, m_student_name);
	strcat(title, " - ");
	strcat(title, SOFTWARE_NAME);

	sqlite3 * db = NULL;
	int db_ret = sqlite3_open("URPScoreHelper.db", &db);
	if (db_ret != SQLITE_OK)
	{
		cout << "Status: 500 Internal Server Error\n";
		Error("打开数据库文件失败，请检查 URPScoreHelper.db 是否存在。");
		return -1;
	}

	// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
	char *query = new char[strlen("SELECT openid FROM URPScoreHelper WHERE id='") + 128 + 1];
	memset(query, 0, strlen("SELECT openid FROM URPScoreHelper WHERE id='") + 128 + 1);
	strcpy(query, "SELECT openid FROM URPScoreHelper WHERE id='");
	strcat(query, m_student_id);
	strcat(query, "';");

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	db_ret = sqlite3_prepare(db, query, strlen(query), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		cout << "Status: 500 Internal Server Error\n";
		char Err_Msg[512] = "<b>数据库准备失败！请确认数据库合法性。</b><p>(";
		strcat(Err_Msg, sqlite3_errmsg(db));
		strcat(Err_Msg, ")</p>");
		Error(Err_Msg);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		delete[]query;
		free(m_lpszHomepage);
		free(m_photo);
		return -1;
	}

	const unsigned char *openid = NULL;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		openid = sqlite3_column_text(stmt, 0);
		break;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	delete[]query;

	fprintf(stdout, header, title);
	if (openid == NULL)
	{
		fprintf(stdout, m_lpszHomepage, SOFTWARE_NAME, m_photo, m_student_name, m_student_id,
			"block", "none", "",
			SOFTWARE_NAME, __DATE__, __TIME__, CGI_SERVER_SOFTWARE);
	}
	else
	{
		fprintf(stdout, m_lpszHomepage, SOFTWARE_NAME, m_photo, m_student_name, m_student_id,
			"none", "block", m_student_id,
			SOFTWARE_NAME, __DATE__, __TIME__, CGI_SERVER_SOFTWARE);
	}
	cout << footer;

	free(m_lpszHomepage);
	free(m_photo);
	return 0;
}

// 处理主页面请求 (GET / /index.cgi)
int parse_index()
{
	int m_iResult = 0;
	bool m_need_update_cookie = false;
	char *m_photo = (char *)malloc(102424);
	ZeroMemory(m_photo, 102424);
	process_cookie(&m_need_update_cookie, m_photo);

	if (strlen(m_photo) != 0)
	{
		if (strcmp(CGI_SCRIPT_NAME, "/main.cgi") == 0)
		{
			parse_main(m_need_update_cookie, m_photo, false);
			return 0;
		}
		cout << "Status: 302 Found\nLocation: main.cgi\n" << GLOBAL_HEADER;
		return 0;
	}
	else
	{
		if (strcmp(CGI_SCRIPT_NAME, "/main.cgi") == 0) // 如果还没登录就请求main.cgi，那就踢回去让他登陆
		{
			cout << "Status: 302 Found\nLocation: index.cgi\n" << GLOBAL_HEADER;
			return 0;
		}
	}

	// 如果是QQ登录回来，则自动填充账号密码。
	char *m_xh = NULL;
	char *m_mm = NULL;
	char *pStr1 = strstr((char *)CGI_QUERY_STRING, "id=");
	if (pStr1 != NULL)
	{
		char *pStr2 = strstr(pStr1 + 3, "&");
		char *id = new char[strlen(CGI_QUERY_STRING)];
		if (pStr2 == NULL)
		{
			right(id, pStr1 + 3, strlen(CGI_QUERY_STRING) - 3);
		}
		else
		{
			mid(id, pStr1 + 3, pStr2 - pStr1 - 3, 0);
		}
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
			m_mm = pass;
		}
	}

	// 置随机数种子，并取得一个随机数，用于获取验证码。
	srand((int)time(0));
	int m_rand = rand();
	char Captcha[256] = { 0 };
	sprintf(Captcha, REQUEST_CAPTCHA, m_rand, JSESSIONID);

	// 发送验证码请求，获取验证码数据。
	char * m_rep_body = (char *)malloc(8192);
	ZeroMemory(m_rep_body, 8192);
	if (!CrawlRequest(Captcha, m_rep_body, 8192, &m_iResult))
	{
		free(m_rep_body);
		return -1;
	}

	// 从返回数据流中获取验证码图片。
	pStr1 = strstr(m_rep_body, "\r\n\r\n");
	if (pStr1 == NULL)
	{
		
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>无法分析验证码响应协议。</p>");
		return -1;
	}
	pStr1 += 4; // 指针后移四位，指向 jpg 开始位置。
	int m_CaptchaLength = m_iResult - (pStr1 - m_rep_body); // 验证码图片的大小

	// 将验证码转化为 base64 编码后的 DataURL，浏览器直接显示，供用户查看。
	char *m_base64 = new char[m_CaptchaLength * 2 + 1];
	base64_encode((const unsigned char *)pStr1, m_base64, m_CaptchaLength);
	char *m_DataURL = new char[m_CaptchaLength * 2 + 24];;
	strcpy(m_DataURL, "data:image/jpg;base64,");
	strcat(m_DataURL, m_base64);
	free(m_rep_body);

	// 读入主页面文件
	FILE *m_file_homepage = fopen(CGI_PATH_TRANSLATED, "rb");
	if (m_file_homepage == NULL)
	{
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>错误：找不到登录页面模板。</p>");
		return -1;
	}
	fseek(m_file_homepage, 0, SEEK_END); // 移到尾
	int m_file_homepage_length = ftell(m_file_homepage); // 获取文件长度
	fseek(m_file_homepage, 0, SEEK_SET); // 重新移回来
	char *m_lpszHomepage = (char *)malloc(m_file_homepage_length + 1);
	ZeroMemory(m_lpszHomepage, m_file_homepage_length + 1);
	if (fread(m_lpszHomepage, m_file_homepage_length, 1, m_file_homepage) != 1) // 将硬盘数据拷至内存
	{
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>无法读取登录页模板内容。</p>");
		fclose(m_file_homepage);
		free(m_lpszHomepage);
		return -1;
	}
	fclose(m_file_homepage); // 关闭文件

	// 输出网页
	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";

	cout << GLOBAL_HEADER;

	fprintf(stdout, header, SOFTWARE_NAME);
	if (m_xh == NULL || m_mm == NULL)
	{
		fprintf(stdout, m_lpszHomepage, g_QueryCount, 
						"输入你的教务系统账号来查询成绩 :)", "block", "", "", m_DataURL, "block", "none",
						SOFTWARE_NAME, __DATE__, __TIME__, CGI_SERVER_SOFTWARE);
	}
	else 
	{
		fprintf(stdout, m_lpszHomepage, g_QueryCount, 
						"QQ登录成功，输入验证码继续吧 :)", "none", m_xh, m_mm, m_DataURL, "none", "block",
						SOFTWARE_NAME, __DATE__, __TIME__, CGI_SERVER_SOFTWARE);
	}
	cout << footer;

	free(m_lpszHomepage);
	delete[]m_base64;
	delete[]m_DataURL;
	return 0;
}

// 处理查询页面请求 (GET /query.cgi)
int parse_query()
{
	bool m_need_update_cookie = false;
	char *m_photo = (char *)malloc(102424);
	ZeroMemory(m_photo, 102424);
	process_cookie(&m_need_update_cookie, m_photo);

	if (strlen(m_photo) == 0) // 还没登陆就丢去登陆。
	{
		cout << "Status: 302 Found\nLocation: index.cgi\n" << GLOBAL_HEADER;
		return 0;
	}

	free(m_photo);

	// 开始查分(本学期)。
	int m_iResult = 0;
	char QUERY_SCORE[512] = { 0 };
	char *m_rep_body = (char *)malloc(81920);
	sprintf( QUERY_SCORE, REQUEST_QUERY_SCORE, CGI_HTTP_COOKIE );
	if (!CrawlRequest(QUERY_SCORE, m_rep_body, 81920, &m_iResult))
	{
		student_logout();
		free(m_rep_body);
		return -1;
	}
	char *m_result = strstr(m_rep_body, "\r\n\r\n");
	if (m_result == NULL)
	{
		student_logout();
		free(m_rep_body);
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>从服务器拉取分数失败。</p>");
		return -1;
	}

	// 优化接受结果，显示查询页面
	parse_friendly_score(m_result);
	free(m_rep_body);

	// 处理完毕。
	
	return 0;
}

// 输出分数页面
void parse_friendly_score(char *p_lpszScore)
{
	char m_Student[64] = { 0 };
	get_student_name(m_Student);

	// 读入分数页面文件
	FILE *m_file_query = fopen(CGI_PATH_TRANSLATED, "rb");
	if (m_file_query == NULL)
	{
		student_logout();
		Error("<p>错误：找不到分数页面模板。</p>");
		return;
	}
	fseek(m_file_query, 0, SEEK_END); // 移到尾
	int m_file_query_length = ftell(m_file_query); // 获取文件长度
	fseek(m_file_query, 0, SEEK_SET); // 重新移回来
	char *m_lpszQuery = (char *)malloc(m_file_query_length + 1);
	ZeroMemory(m_lpszQuery, m_file_query_length + 1);
	if (fread(m_lpszQuery, m_file_query_length, 1, m_file_query) != 1) // 将硬盘数据拷至内存
	{
		student_logout();
		Error("<p>无法读取分数页模板内容。</p>");
		fclose(m_file_query);
		free(m_lpszQuery);
		return;
	}
	fclose(m_file_query); // 关闭文件

	char *m_query_not_reg = strstr(p_lpszScore, "没有注册");
	if (m_query_not_reg != NULL)
	{
		free(m_lpszQuery);
		cout << "Status: 403 Forbidden\n";

		char *m_original_str = "<p><b>亲爱的%s，系统君说你本学期还没有电子注册 0.0</b></p><p>我可以施展法术，\
一键帮你在教务系统注册哦~</p><p>--&gt; 点按下方按钮，直达查分界面 :P &lt;--</p>\
<div class=\"col-100\"><a href=\"query.cgi?act=system_registration\" class=\"button button-big but\
ton-fill button-success\">一键注册</a></div>";

		char m_output_str[1024] = { 0 };
		sprintf(m_output_str, m_original_str, m_Student);
		Error(m_output_str);
		return;
	}
	if (strcmp(CGI_QUERY_STRING, "order=tests") == 0)
	{
		int m_iResult = 0;
		char Req[512] = { 0 };
		char *m_rep_body = (char *)malloc(204800);
		sprintf(Req, GET_SMALL_TEST_SCORE, CGI_HTTP_COOKIE);
		if (!CrawlRequest(Req, m_rep_body, 204800, &m_iResult))
		{
			free(m_rep_body);
			return;
		}
		char *m_result = strstr(m_rep_body, "<table cellpadding=\"0\" width=\"100%\" class=\"displayTag\" cellspacing=\"1\" border=\"0\" id=\"user\">");
		if (m_result == NULL)
		{
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p><b>从服务器拉取分数失败。(BeginOfTable)</b></p><p>教务君可能月线繁忙，666 请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		m_result += 93;
		char *m_prep = (char *)malloc(205900);
		strcpy(m_prep, "<div id=\"list_page\">");
		char *m_end_body = strstr(m_result, "</table>");
		if (m_result == NULL)
		{
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>从服务器拉取分数失败。(EndOfBodyNotFound)</p>");
			return;
		}
		m_result -= 93;
		cout << GLOBAL_HEADER;
		char m_before[512] = { 0 };
		sprintf(m_before, "<a name=\"qb_731\"></a><table width=\"100%%\" border=\"0\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td class=\"Linetop\"></td></tr></tbody></table><table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\"><tbody><tr><td width=\"100%%\"><table border=\"0\" align=\"left\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td>&nbsp;</td><td valign=\"middle\">&nbsp;<b>%s</b> &nbsp;</td></tr></tbody></table></td></tr></tbody></table>", "成绩清单（月考/期中/补考/缓考/清考）");
		*(m_end_body + 8) = '<';
		*(m_end_body + 9) = '/';
		*(m_end_body + 10) = 'd';
		*(m_end_body + 11) = 'i';
		*(m_end_body + 12) = 'v';
		*(m_end_body + 13) = '>';
		*(m_end_body + 14) = '\0';

		strcat(m_prep, m_before);
		strcat(m_prep, m_result);

		char title[256] = { 0 };
		strcpy(title, m_Student);
		strcat(title, "的考试成绩 - ");
		strcat(title, SOFTWARE_NAME);

		fprintf(stdout, header, title);
		fprintf(stdout, m_lpszQuery, m_Student, "", "", "", "", m_prep);
		cout << footer;

		fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
		fclose(g_fQueryCount);
		free(m_lpszQuery);
		free(m_prep);
		// free(m_result); BUG CAN NOT RELEASE!
		return;
	}
	if (strcmp(CGI_QUERY_STRING, "order=passed") == 0)
	{
		//free(p_lpszScore);
		int m_iResult = 0;
		char Req[512] = { 0 };
		char *m_rep_body = (char *)malloc(204800);
		sprintf(Req, GET_GRADE_BY_QBINFO, CGI_HTTP_COOKIE);
		if (!CrawlRequest(Req, m_rep_body, 204800, &m_iResult))
		{
			free(m_rep_body);
			return;
		}
		char *m_result = strstr(m_rep_body, "<body leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\" style=\"overflow:auto;\">");
		if (m_result == NULL)
		{
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p><b>从服务器拉取分数失败。(BeginOfRet)</b></p><p>教务君可能月线繁忙，666 请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		m_result += 92;
		char *m_prep = (char *)malloc(205200);
		strcpy(m_prep, "<div id=\"list_page\">");
		char *m_end_body = strstr(m_result, "</body>");
		if (m_result == NULL)
		{
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>从服务器拉取分数失败。(EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		char title[256] = { 0 };
		strcpy(title, m_Student);
		strcat(title, "的各科通过成绩 - ");
		strcat(title, SOFTWARE_NAME);

		fprintf(stdout, header, title);
		fprintf(stdout, m_lpszQuery, m_Student, "", " active", "", "", m_prep);
		cout << footer;

		fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
		fclose(g_fQueryCount);
		free(m_lpszQuery);
		free(m_prep);
		// free(m_result); BUG CAN NOT RELEASE!
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=byplan") == 0)
	{
		//free(p_lpszScore);
		int m_iResult = 0;
		char Req[512] = { 0 };
		char *m_rep_body = (char *)malloc(204800);
		sprintf(Req, GET_GRADE_BY_PLAN, CGI_HTTP_COOKIE);
		if (!CrawlRequest(Req, m_rep_body, 204800, &m_iResult))
		{
			free(m_rep_body);
			return;
		}
		char *m_result = strstr(m_rep_body, "<body leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\" style=\"overflow:auto;\">");
		if (m_result == NULL)
		{
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p><b>从服务器拉取分数失败。(BeginOfRet)</b></p><p>教务君可能月线繁忙，666 请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		m_result += 92;
		char *m_prep = (char *)malloc(205200);
		strcpy(m_prep, "<div id=\"list_page\">");
		char *m_end_body = strstr(m_result, "</body>");
		if (m_result == NULL)
		{
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>从服务器拉取分数失败。(EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		char title[256] = { 0 };
		strcpy(title, m_Student);
		strcat(title, "的专业方案成绩 - ");
		strcat(title, SOFTWARE_NAME);

		fprintf(stdout, header, title);
		fprintf(stdout, m_lpszQuery, m_Student, "", "", " active", "", m_prep);
		cout << footer;

		fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
		fclose(g_fQueryCount);
		free(m_lpszQuery);
		free(m_prep);
		// free(m_result); BUG CAN NOT RELEASE!
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=failed") == 0)
	{
		//free(p_lpszScore);
		int m_iResult = 0;
		char Req[512] = { 0 };
		char *m_rep_body = (char *)malloc(204800);
		sprintf(Req, GET_GRADE_BY_FAILED, CGI_HTTP_COOKIE);
		if (!CrawlRequest(Req, m_rep_body, 204800, &m_iResult))
		{
			free(m_rep_body);
			return;
		}
		char *m_result = strstr(m_rep_body, "<table width=\"100%\"  border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\">");
		if (m_result == NULL)
		{
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p><b>从服务器拉取分数失败。(BeginOfRet)</b></p><p>教务君可能月线繁忙，666 请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		m_result = strstr(m_result + 92, "<table width=\"100%\"  border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\">");
		if (m_result == NULL)
		{
			
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p><b>从服务器拉取分数失败。(MidOfRet-Table)</b></p><p>教务君可能月线繁忙，666 请稍候再试。</p><p>如果月线正忙，或存在数据显示遗漏，多刷新几次即可。</p>");
			return;
		}
		char *m_prep = (char *)malloc(205200);
		strcpy(m_prep, "<div id=\"list_page\">");
		char *m_end_body = strstr(m_result, "</body>");
		if (m_result == NULL)
		{
			free(m_rep_body);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>从服务器拉取分数失败。(EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		char title[256] = { 0 };
		strcpy(title, m_Student);
		strcat(title, "的不及格成绩 - ");
		strcat(title, SOFTWARE_NAME);

		fprintf(stdout, header, title);
		fprintf(stdout, m_lpszQuery, m_Student, "", "", "", " active", m_prep);
		cout << footer;

		fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
		fclose(g_fQueryCount);
		free(m_lpszQuery);
		free(m_prep);
		// free(m_result); BUG CAN NOT RELEASE!
		return;
	}

	// 定位到第一项成绩
	char *pStr1 = strstr(p_lpszScore,"<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
	if (pStr1 == NULL)
	{
		free(m_lpszQuery);
		Error("<p><b>额，分数页面神隐了 0.0。</b></p><p>这可能是因为新的学期开始了，上次考试成绩存档了哦~</p><p>建议看看已通过课程成绩和按专业成绩，这里面可以看到历年成绩和所得学分哦~</p>");
		return;
	}

	bool m_success = false;
	char m_Output[81920] = { 0 };
	char *pStr2 = NULL;
	char *pStr3 = NULL;

	// 循环获取每一项成绩信息

	double m_Total_xuefen = 0.0;
	double m_Total_pointsxxuefen = 0.0;

	while (pStr1 != NULL) 
	{
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

		pStr2 = pStr3;
		pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subXuefen[128] = { 0 };
		mid(m_subXuefen, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_subXuefen) == 0) sprintf(m_subXuefen, "暂无数据");

		pStr2 = pStr3;
		pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuigaofen[128] = { 0 };
		mid(m_subzuigaofen, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_subzuigaofen) == 0) sprintf(m_subzuigaofen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuidifen[128] = { 0 };
		mid(m_subzuidifen, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_subzuidifen) == 0) sprintf(m_subzuidifen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subjunfen[128] = { 0 };
		mid(m_subjunfen, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_subjunfen) == 0) sprintf(m_subjunfen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subchengji[256] = { 0 };
		mid(m_subchengji, pStr2, pStr3 - pStr2 - 19, 19);
		if (strstr(m_subchengji, "优秀") != NULL)
		{
			strcpy(m_subchengji,"95");
		}
		if (strstr(m_subchengji, "良好") != NULL)
		{
			strcpy(m_subchengji, "85");
		}
		if (strstr(m_subchengji, "中等") != NULL)
		{
			strcpy(m_subchengji, "75");
		}
		if (strstr(m_subchengji, "及格") != NULL)
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
		//if (atoi(m_subchengji) == 0) strcpy(m_subchengji, "暂无数据");
		if (atof(m_subchengji) < 60) 
		{

			char m_completecj[256] = "<b style=\"color:#f6383a\">";
			strcat(m_completecj, m_subchengji);
			strcat(m_completecj, "</b>");
			ZeroMemory(m_subchengji, 256);
			strcpy(m_subchengji, m_completecj);
		}

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_submingci[128] = { 0 };
		mid(m_submingci, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_submingci) == 0) sprintf(m_submingci, "暂无数据");

		char m_StrTmp[10240] = { 0 };
		sprintf(m_StrTmp, SCORE_TEMPLATE, m_subName, m_subchengji, m_subjunfen, m_subzuigaofen, m_subzuidifen,
				m_submingci, m_subXuefen);
		strcat(m_Output, m_StrTmp);

		// （分数x学分）全都加起来/总学分 = 加权分，排除体育和课程设计
		if (strstr(m_subName, "体育") == NULL && strstr(m_subName, "军事训练") == NULL 
			&& strstr(m_subName, "实践") == NULL)
		{
			float m_xuefen = atof(m_subXuefen);
			float m_chengji = atof(m_subchengji);
			if (m_xuefen != 0) // 排0学分...，并统计总学分
			{
				m_Total_xuefen += m_xuefen;
			}
			if (m_chengji != 0)
			{
				double m_pointsxxuefen = m_xuefen * m_chengji;
				if (m_pointsxxuefen != 0)
				{
					m_Total_pointsxxuefen += m_pointsxxuefen;
				}
			}
		}
		m_success = true; // 查到一个算一个
		pStr1 = strstr(pStr3, "<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
	}

	// 假如发生了错误
	if (!m_success) 
	{
		free(m_lpszQuery);
		Error("<p>内个，发生意外错误啦。</p>");
		return;
	}

	// 填充返回页面
	if (m_Total_pointsxxuefen != 0 || m_Total_xuefen != 0)
	{
		char m_jiaquanfen[81920] = { 0 };
		sprintf(m_jiaquanfen, "<div id=\"i_total\"><p>本学期加权综合分（评优、排名依据，供参考）：</p><center>%.1f</center></div>", 
				m_Total_pointsxxuefen / m_Total_xuefen);
		strcat(m_jiaquanfen, m_Output);
		ZeroMemory(m_Output, 81920);
		strcpy(m_Output, m_jiaquanfen);
	}

	char m_query_time[512] = { 0 };
	sprintf(m_query_time, "<center>本次查询耗时 %.2f 秒</center>", (double)((GetTickCount() - g_start_time) / 1000));
	strcat(m_Output, m_query_time);

	cout << GLOBAL_HEADER;

	char title[256] = { 0 };
	strcpy(title, m_Student);
	strcat(title, "的本期成绩 - ");
	strcat(title, SOFTWARE_NAME);

	fprintf(stdout, header, title);
	fprintf(stdout, m_lpszQuery, m_Student, " active", "", "", "", m_Output);

	cout << footer;
	
	free(m_lpszQuery);
	fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
	fclose(g_fQueryCount);
}

// 获取学生姓名
void get_student_name(char *p_lpszBuffer)
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		return;
	}

	int m_iResult = 0;
	char * m_rep_header = (char *)malloc(10240);
	ZeroMemory(m_rep_header, 10240);
	char GET_TOP[1024] = { 0 };
	sprintf(GET_TOP, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE);
	if (!CrawlRequest(GET_TOP, m_rep_header, 10240, &m_iResult))
	{
		free(m_rep_header);
		return;
	}

	// 拉取学生姓名
	char *pStr1 = strstr(m_rep_header, "<td class=\"fieldName\">姓名:&nbsp;</td>");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		return;
	}
	pStr1 = strstr(pStr1 + 39, "<td>");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		return;
	}
	char *pStr2 = strstr(pStr1 + 5,"</td>");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		return;
	}
	mid(p_lpszBuffer, pStr1, pStr2 - pStr1 - 4, 4);
	free(m_rep_header);
}

// 获取学生账号
void get_student_id(char *p_lpszBuffer)
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		return;
	}

	int m_iResult = 0;
	char * m_rep_header = (char *)malloc(8192);
	ZeroMemory(m_rep_header, 8192);
	char GET_TOP[1024] = { 0 };
	sprintf(GET_TOP, REQUEST_TOP, CGI_HTTP_COOKIE);
	if (!CrawlRequest(GET_TOP, m_rep_header, 8192, &m_iResult))
	{
		free(m_rep_header);
		strcpy(p_lpszBuffer, "\0");
		return;
	}

	// 拉取学生姓名
	char *pStr1 = strstr(m_rep_header, "当前用户:");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		strcpy(p_lpszBuffer, "\0");
		return;
	}
	char *pStr2 = strstr(pStr1 + 8, "(");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		strcpy(p_lpszBuffer, "\0");
		return;
	}
	mid(p_lpszBuffer, pStr1, pStr2 - pStr1 - 9, 9);
	free(m_rep_header);
}

// 教务系统电子注册 (GET /query.cgi?act=system_registration)
int system_registration()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		cout << "Status: 302 Found\n" << "Location: index.cgi\n" << GLOBAL_HEADER;
		cout << GLOBAL_HEADER;
		return -1;
	}

	int m_iResult = 0;

	// 发送请求，获取电子注册信息。
	char * m_rep_header = (char *)malloc(10240);
	ZeroMemory(m_rep_header, 10240);
	char m_req[1024] = { 0 };
	sprintf(m_req, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE);
	if (!CrawlRequest(m_req, m_rep_header, 10240, &m_iResult))
	{
		free(m_rep_header);
		student_logout();
		
		return -1;
	}

	// 查找需要电子注册的学期信息。
	char *pStr1 = strstr(m_rep_header, "selected>");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (1)</p>");
		student_logout();
		
		return -1;
	}
	pStr1 -= 70;
	char *pStr2 = strstr(pStr1, "<option value=\"");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (2)</p>");
		student_logout();
		
		return -1;
	}
	pStr1 = pStr2;
	pStr2 = strstr(pStr1 + 16, "\"");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (3)</p>");
		student_logout();
		
		return -1;
	}

	char m_regval[4096] = { 0 };
	mid(m_regval, pStr1, pStr2 - pStr1 - 15, 15);
	free(m_rep_header);

	// 填充电子注册信息
	char m_post_reg_info[4096] = "zxjxjhh=";
	strcat(m_post_reg_info, m_regval);
	int m_post_reg_info_length = strlen(m_post_reg_info);

	// 填充注册请求
	char m_post_req[8192] = { 0 };
	sprintf(m_post_req, REQUEST_POST_REGISTER_INTERFACE, m_regval, CGI_HTTP_COOKIE, m_post_reg_info_length, 
			m_post_reg_info);

	// 开始电子注册
	m_rep_header = (char *)malloc(10240);
	ZeroMemory(m_rep_header, 10240);
	if (!CrawlRequest(m_post_req, m_rep_header, 10240, &m_iResult))
	{
		student_logout();
		free(m_rep_header);
		
		return -1;
	}

	// 检查是否注册成功。
	pStr1 = strstr(m_rep_header, "注册成功");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		student_logout();
		
		Error("<p>不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (4)</p>");
		return -1;
	}

	// 注册成功了。
	free(m_rep_header);

	// 为刚电子注册的同学查询成绩
	char QUERY_SCORE[512] = { 0 };
	m_rep_header = (char *)malloc(81920);
	sprintf(QUERY_SCORE, REQUEST_QUERY_SCORE, CGI_HTTP_COOKIE);
	if (!CrawlRequest(QUERY_SCORE, m_rep_header, 81920, &m_iResult))
	{
		free(m_rep_header);
		
		return -1;
	}
	char *m_result = strstr(m_rep_header, "\r\n\r\n");
	if (m_result == NULL)
	{
		student_logout();
		free(m_rep_header);
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>从服务器拉取分数失败。</p>");
		
		return -1;
	}

	// 优化接受结果，显示查询页面
	parse_friendly_score(m_result);
	free(m_rep_header);
	
	// 完事~
	
	return 0;
}

// 登录学生
bool student_login(char *p_xuehao, char *p_password, char *p_captcha)
{
	// 发送登陆请求。
	int m_iResult = 0;
	char * m_rep_body = (char *)malloc(40960);
	ZeroMemory(m_rep_body, 40960);
	char POST_LOGIN[1024] = { 0 };
	char *m_origin = "zjh1=&tips=&lx=&evalue=&eflag=&fs=&dzslh=&zjh=%s&mm=%s&v_yzm=%s";
	char m_padding[1024] = { 0 };
	sprintf(m_padding, m_origin, p_xuehao, p_password, p_captcha);
	int m_ContentLength = strlen(m_padding); // TODO: 这里不用加莫名其妙的结束长度
	sprintf(POST_LOGIN, REQUEST_LOGIN, m_ContentLength, CGI_HTTP_COOKIE, p_xuehao, p_password, p_captcha);
	if (!CrawlRequest(POST_LOGIN, m_rep_body, 40960, &m_iResult))
	{
		free(m_rep_body);
		return false;
	}

	// 拉取登录结果。
	char *m_result = strstr(m_rep_body, "\r\n\r\n");
	if (m_result == NULL)
	{
		
		free(m_rep_body);
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>从服务器拉取登录结果失败。</p>");
		return false;
	}

	// 处理登录结果。
	char *m_login_not_auth = strstr(m_result, "证件号");
	if (m_login_not_auth != NULL)
	{
		
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>证件号或密码不对啊，大佬。 TAT。</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "密码不正确");
	if (m_login_not_auth != NULL)
	{
		
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p><b>学号或密码不对啊。</b></p><p>如果你曾修改过教务系统的账号密码，请使用新密码再试一试。</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "验证码错误");
	if (m_login_not_auth != NULL)
	{
		
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>验证码不对啊，大佬。 TAT。</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "数据库");
	if (m_login_not_auth != NULL)
	{
		
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>教务系统君说数据库繁忙 :P</p><p>对于<b>数据库跑路</b>问题，那就等等先咯~</p>");
		return false;
	}
	char *m_login_success = strstr(m_result, "学分制综合教务");
	if (m_login_success == NULL)
	{
		
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>天呐。发生了谜一般的问题！教务系统神隐了 0.0</p><p>建议你稍候再试试吧。</p>");
		return false;
	}

	// 这里表示登录成功，应该写入数据库了。
	sqlite3 * db = NULL;
	int db_ret = sqlite3_open("URPScoreHelper.db", &db);
	if (db_ret != SQLITE_OK)
	{
		student_logout();
		cout << "Status: 500 Internal Server Error\n";
		Error("打开数据库文件失败，请检查 URPScoreHelper.db 是否存在。");
		return false;
	}

	// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
	char *query = new char[strlen("SELECT id FROM URPScoreHelper WHERE id='") + 128 + 1];
	memset(query, 0, strlen("SELECT id FROM URPScoreHelper WHERE id='") + 128 + 1);
	strcpy(query, "SELECT id FROM URPScoreHelper WHERE id='");
	strcat(query, p_xuehao);
	strcat(query, "';");

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	db_ret = sqlite3_prepare(db, query, strlen(query), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		student_logout();
		cout << "Status: 500 Internal Server Error\n";
		char Err_Msg[512] = "<b>数据库准备失败！请确认数据库合法性。</b><p>(";
		strcat(Err_Msg, sqlite3_errmsg(db));
		strcat(Err_Msg, ")</p>");
		Error(Err_Msg);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		delete[]query;
		return false;
	}

	const unsigned char *id = NULL;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		id = sqlite3_column_text(stmt, 0);
		break;
	}

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	delete[]query;

	if (id == NULL) // 无记录，则写入数据库
	{
		char *query = new char[strlen("INSERT INTO URPScoreHelper (id, password, openid, lastlogin) VALUES (") + 36 + 36 + 64 + 128 + 1];
		memset(query, 0, strlen("INSERT INTO URPScoreHelper (id, password, openid, lastlogin) VALUES (") + 36 + 36 + 64 + 128 + 1);
		strcpy(query, "INSERT INTO URPScoreHelper (id, password, openid, lastlogin) VALUES ('");
		strcat(query, p_xuehao);
		strcat(query, "', '");
		strcat(query, p_password);
		strcat(query, "', NULL, '");
		char m_time[128] = { 0 };
		get_time(m_time);
		strcat(query, m_time);
		strcat(query, "');");

		char **db_Result = NULL;
		sqlite3_stmt *stmt;
		db_ret = sqlite3_prepare(db, query, strlen(query), &stmt, 0);

		if (db_ret != SQLITE_OK)
		{
			student_logout();
			cout << "Status: 500 Internal Server Error\n";
			char Err_Msg[1024] = "<b>很抱歉，登录失败。</b><p>数据库错误 (";
			strcat(Err_Msg, sqlite3_errmsg(db));
			strcat(Err_Msg, ")</p>");
			Error(Err_Msg);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			delete[]query;
			return false;
		}
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		delete[]query;
	}
	else // 为成功登录的学生更新记录
	{
		sqlite3 * db = NULL;
		int db_ret = sqlite3_open("URPScoreHelper.db", &db);
		if (db_ret != SQLITE_OK)
		{
			student_logout();
			cout << "Status: 500 Internal Server Error\n";
			Error("打开数据库文件失败，请检查 URPScoreHelper.db 是否存在。");
			return false;
		}

		// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128) text lastlogin(64)。
		char *query = new char[strlen("UPDATE URPScoreHelper SET password=") + 128 + strlen(" lastlogin= WHERE id='") + 64 + 36 + 3 + 1];
		memset(query, 0, strlen("UPDATE URPScoreHelper SET password=") + 128 + strlen(" lastlogin= WHERE id='") + 64 + 36 + 3 + 1);
		strcpy(query, "UPDATE URPScoreHelper SET password='");
		char m_time[128] = { 0 };
		get_time(m_time);
		strcat(query, p_password);
		strcat(query, "', lastlogin='");
		strcat(query, m_time);
		strcat(query, "' WHERE id = '");
		strcat(query, p_xuehao);
		strcat(query, "';");

		char **db_Result = NULL;
		sqlite3_stmt *stmt;
		db_ret = sqlite3_prepare(db, query, strlen(query), &stmt, 0);

		if (db_ret != SQLITE_OK)
		{
			student_logout();
			cout << "Status: 500 Internal Server Error\n";
			char Err_Msg[512] = "<b>登录数据库记录失败，请稍后再试。</b><p>(";
			strcat(Err_Msg, sqlite3_errmsg(db));
			strcat(Err_Msg, ")</p>");
			Error(Err_Msg);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			delete[]query;
			return false;
		}

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			break;
		}

		sqlite3_finalize(stmt);
		delete[]query;
	}

	// 至此，学生登录成功，释放资源。
	sqlite3_close(db);
	free(m_rep_body);
	return true;
}

// 登出学生
void student_logout()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0) return;

	int m_iResult = 0;
	char m_logout[10240] = { 0 };
	sprintf(m_logout, REQUEST_LOGOUT, CGI_HTTP_COOKIE);
	char *m_outbuffer = (char *)malloc(10240);
	CrawlRequest(m_logout, m_outbuffer, 10240, &m_iResult);
	free(m_outbuffer);
}

// 快速查询入口 (/QuickQuery.cgi)
void parse_QuickQuery_Intro()
{
	bool m_need_update_cookie = false;
	process_cookie(&m_need_update_cookie, NULL);

	// 读入页面文件
	FILE *m_file_query = fopen(CGI_PATH_TRANSLATED, "rb");
	if (m_file_query == NULL)
	{
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>错误：找不到免密查询模板。</p>");
		return;
	}
	fseek(m_file_query, 0, SEEK_END); // 移到尾
	int m_file_query_length = ftell(m_file_query); // 获取文件长度
	fseek(m_file_query, 0, SEEK_SET); // 重新移回来
	char *m_lpszQuery = (char *)malloc(m_file_query_length + 1);
	ZeroMemory(m_lpszQuery, m_file_query_length + 1);
	if (fread(m_lpszQuery, m_file_query_length, 1, m_file_query) != 1) // 将硬盘数据拷至内存
	{
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>无法读取免密查询模板内容。</p>");
		fclose(m_file_query);
		free(m_lpszQuery);
		return;
	}
	fclose(m_file_query); // 关闭文件

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
	cout << GLOBAL_HEADER;

	char title[512] = { 0 };
	strcat(title, "免密成绩查询 - ");
	strcat(title, SOFTWARE_NAME);

	fprintf(stdout, header, title);
	fprintf(stdout, m_lpszQuery, g_QueryCount);

	cout << footer;

	free(m_lpszQuery);
}

// 免密查询结果 (/query.cgi?act=QuickQuery)
void parse_QuickQuery_Result()
{
	bool m_need_update_cookie = false;
	process_cookie(&m_need_update_cookie, NULL);

	// 读入分数页面文件
	FILE *m_file_query = fopen(CGI_PATH_TRANSLATED, "rb");
	if (m_file_query == NULL)
	{
		Error("<p>错误：找不到分数页面模板。</p>");
		return;
	}
	fseek(m_file_query, 0, SEEK_END); // 移到尾
	int m_file_query_length = ftell(m_file_query); // 获取文件长度
	fseek(m_file_query, 0, SEEK_SET); // 重新移回来
	char *m_lpszQuery = (char *)malloc(m_file_query_length + 1);
	ZeroMemory(m_lpszQuery, m_file_query_length + 1);
	if (fread(m_lpszQuery, m_file_query_length, 1, m_file_query) != 1) // 将硬盘数据拷至内存
	{
		Error("<p>无法读取分数页模板内容。</p>");
		fclose(m_file_query);
		free(m_lpszQuery);
		return;
	}
	fclose(m_file_query); // 关闭文件

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		cout << "Status: 500 Internal Server Error\n";
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>发生错误，POST 数据长度异常。</p>");
		
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);	// TORESEARCH
	if (fgets(m_post_data, m_post_length + 1, stdin) == NULL)
	{
		cout << "Status: 500 Internal Server Error\n";
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>发生错误，POST 数据拉取失败。</p>");
		
		return;
	}

	// 获取学号
	char *pStr1 = strstr(m_post_data, "xh=");
	if (pStr1 == NULL)
	{
		
		free(m_post_data);
		cout << "Status: 500 Internal Server Error\n";
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>无法获取学号信息。</p>");
		return;
	}
	char *pStr2 = strstr(pStr1 + 3, "&");
	char m_xuehao[128] = { 0 };
	right(m_xuehao, pStr1, strlen(pStr1) - 3);
	replace_string(m_xuehao, "%0D%0A", "|");
	char *p = strtok(m_xuehao, "|");
	char *m_xh[10] = { NULL };
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
		cout << "Status: 500 Internal Server Error\n";
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>输入的学号个数存在问题，请确认！</p>");
		return;
	}

	char m_list[102400] = { 0 };
	char m_xxmz[128] = { 0 };
	free(m_post_data);

	for (int xh_index = 0; xh_index < m_xhgs; xh_index++)
		{
			if (strlen(m_xh[xh_index]) != 9)
			{
				
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>输入的学号中有长度存在问题，请确认！</p>");
				return;
			}

			char m_query_param[1024] = { 0 };
			sprintf(m_query_param, "LS_XH=%s", m_xh[xh_index]);
			strcat(m_query_param, "&resultPage=http%3A%2F%2Fjw0.yzu.edu.cn%3A80%2FreportFiles%2Fcj%2Fcj_zwcjd.jsp%3F");
			char m_query_request[2048] = { 0 };
			sprintf(m_query_request, REQUEST_SET_REPORT_PARAMS, CGI_HTTP_COOKIE, strlen(m_query_param));
			strcat(m_query_request, m_query_param);

			int m_ilength = 0;
			char *m_lpvBuffer = (char *)malloc(4096);
			ZeroMemory(m_lpvBuffer, 4096);
			if (!CrawlRequest(m_query_request, m_lpvBuffer, 4096, &m_ilength))
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>投递免密查询请求失败。</p><p>请确认教务系统是可用的。</p>");
				
				free(m_lpvBuffer);
				return;
			}

			pStr1 = strstr(m_lpvBuffer, "&reportParamsId=");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>获取报表ID错误。(1)</p>");
				
				free(m_lpvBuffer);
				return;
			}
			pStr2 = strstr(pStr1 + 16, "\n");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>获取报表ID错误。(2)</p>");
				
				free(m_lpvBuffer);
				return;
			}
			char m_paramsID[512] = { 0 };
			mid(m_paramsID, pStr1 + 16, pStr2 - pStr1 - 16, 0);
			pStr1 = NULL;
			pStr2 = NULL;
			free(m_lpvBuffer);

			char m_query_report[512] = { 0 };
			sprintf(m_query_report, REQUEST_REPORT_FILES, m_paramsID, CGI_HTTP_COOKIE);
			m_lpvBuffer = (char *)malloc(40960);
			ZeroMemory(m_lpvBuffer, 40960);
			if (!CrawlRequest(m_query_report, m_lpvBuffer, 40960, &m_ilength))
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>通过ID免密查询失败。</p><p>发生了天知道的错误。</p>");
				
				free(m_lpvBuffer);
				return;
			}

			pStr1 = strstr(m_lpvBuffer, "Exception: ");
			if (pStr1 != NULL)
			{
				pStr2 = strstr(pStr1, "at");
				if (pStr2 != NULL)
				{
					cout << "Status: 500 Internal Server Error\n";
					if (m_need_update_cookie)
						cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
					char m_Exception[512] = { 0 };
					mid(m_Exception, pStr1 + 11, pStr2 - pStr1 - 11, 0);
					char m_ExceptionMsg[512] = "<p><b>教务系统抛出了如下错误...</b></p><p>";
					strcat(m_ExceptionMsg, m_Exception);
					strcat(m_ExceptionMsg, "</p>");
					Error(m_ExceptionMsg);
					
					free(m_lpvBuffer);
					return;
				}
			}
			pStr1 = NULL;
			pStr2 = NULL;

			pStr1 = strstr(m_lpvBuffer, "com.runqian.report.view.text.TextFileServlet");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>免密查询返回参数错误。(1)</p>");
				
				free(m_lpvBuffer);
				return;
			}
			pStr2 = strstr(pStr1 + 46, "\";");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>免密查询返回参数错误。(2)</p>");
				
				free(m_lpvBuffer);
				return;
			}
			char m_txt_req_path[512] = { 0 };
			mid(m_txt_req_path, pStr1 + 45, pStr2 - pStr1 - 45, 0);
			free(m_lpvBuffer);

			char m_query_score[512] = { 0 };
			sprintf(m_query_score, REQUEST_TXT_SCORES, m_txt_req_path, m_paramsID, CGI_HTTP_COOKIE);
			m_lpvBuffer = (char *)malloc(40960);
			ZeroMemory(m_lpvBuffer, 40960);
			if (!CrawlRequest(m_query_score, m_lpvBuffer, 40960, &m_ilength))
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>接受分数信息失败！</p>");
				
				free(m_lpvBuffer);
				return;
			}

			pStr1 = strstr(m_lpvBuffer, "姓名\t");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>学生姓名获取失败！(1)</p>");
				
				free(m_lpvBuffer);
				return;
			}
			pStr2 = strstr(pStr1 + 4, "\t\t");
			if (pStr2 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>学生姓名获取失败！(2)</p>");
				
				free(m_lpvBuffer);
				return;
			}

			if ((pStr2 - pStr1) <= 4)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error, 
					"<p><b>呃，获取失败了。请确认所输信息是正确的。</b></p><p>发生错误的学号: %s</p>", 
					m_xh[xh_index]);
				Error(m_friendly_error);
				
				free(m_lpvBuffer);
				return;
			}

			mid(m_xxmz, pStr1 + 4, pStr2 - pStr1 - 5, 1);
			if (strlen(m_xxmz) < 2)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					"<p><b>获取信息失败，请确认输入正确。</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				
				free(m_lpvBuffer);
				return;
			}
			char m_xxmz_html[128] = { 0 };
			sprintf(m_xxmz_html, "<div class=\"content-block-title\">%s</div>", m_xxmz);

			pStr1 = NULL;
			pStr2 = NULL;
			pStr1 = strstr(m_lpvBuffer, "考试时间\t\t\n");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					"<p><b>接受到的报表存在问题。</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				
				free(m_lpvBuffer);
				return;
			}

			if (strlen(m_lpvBuffer) <= 800)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					"<p><b>收到的报表大小存在问题。</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				
				free(m_lpvBuffer);
				return;
			}

			pStr1 += 11;
			pStr2 = strstr(pStr1, "\t\t\t\t");
			bool m_success = true;
			strcat(m_list, m_xxmz_html);
			strcat(m_list, "<div class=\"list-block\"><ul>");
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
				if (strstr(m_lb, "重修") != NULL)
				{
					char m_kcmz_cx[256] = { 0 };
					strcat(m_kcmz_cx, "[重修] ");
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

			free(m_lpvBuffer);

			if (!m_success)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>抱歉，免密查询失败，请稍后再试。</p>");
				
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
				if (m_interval > 90)
				{
					if (m_test_info[i].date == m_max_date)
					{
						char m_temp[1024] = { 0 };
						sprintf(m_temp, QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
						strcat(m_list, m_temp);
					}
				}
				else if (m_test_info[i].date == m_max_date || m_test_info[i].date == m_secondary_max)
				{
					char m_temp[1024] = { 0 };
					sprintf(m_temp, QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
					strcat(m_list, m_temp);
				}
			}
			strcat(m_list, "</ul></div>");
		}

		cout << GLOBAL_HEADER;

		char m_query_time[512] = { 0 };
		sprintf(m_query_time, "<center>本次查询耗时 %.2f 秒</center>", (double)((GetTickCount() - g_start_time) / 1000));
		strcat(m_list, m_query_time);

		if (m_xhgs > 1)
		{
			char title[512] = { 0 };
			strcat(title, "多人查询 - 免密成绩查询 - ");
			strcat(title, SOFTWARE_NAME);

			fprintf(stdout, header, title);
			fprintf(stdout, m_lpszQuery, "多人查询", "\" style=\"display:none", "\" style=\"display:none", "\" style=\"display:none", "\" style=\"display:none", m_list);
		}
		else
		{
			char title[512] = { 0 };
			strcat(title, m_xxmz);
			strcat(title, " - 免密成绩查询 - ");
			strcat(title, SOFTWARE_NAME);

			fprintf(stdout, header, title);
			fprintf(stdout, m_lpszQuery, m_xxmz, "\" style=\"display:none", "\" style=\"display:none", "\" style=\"display:none", "\" style=\"display:none", m_list);
		}
		cout << footer;
		g_QueryCount = g_QueryCount + m_xhgs;
		fprintf(g_fQueryCount, "%ld", g_QueryCount);
		fclose(g_fQueryCount);
}

// QQ账号绑定入口与解绑逻辑 (/OAuth2Assoc.cgi)
void OAuth2_Association(bool isPOST)
{
	char *CGI_QUERY_STRING = getenv("QUERY_STRING");
	if (CGI_QUERY_STRING == NULL)
	{
		cout << "Status: 500 Internal Server Error\n";
		Error("参数错误 (Null QUERY_STRING)");
		return;
	}

	// 解除绑定逻辑
	char *pStr1 = strstr(CGI_QUERY_STRING, "release=");
	if (pStr1 != NULL)
	{
		char student_id[36] = { 0 };
		get_student_id(student_id);
		if (student_id == NULL)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("非法操作！ (尚未登录)");
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
			cout << "Status: 500 Internal Server Error\n";
			Error("非法操作！ (身份错误)");
			return;
		}

		sqlite3 * db = NULL;
		int db_ret = sqlite3_open("URPScoreHelper.db", &db);
		if (db_ret != SQLITE_OK)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("打开数据库文件失败，请检查 URPScoreHelper.db 是否存在。");
			return;
		}

		// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
		/*char *query = new char[strlen("DELETE FROM URPScoreHelper WHERE id='") + 36 + 1];
		memset(query, 0, strlen("DELETE FROM URPScoreHelper WHERE id='") + 36 + 1);
		strcpy(query, "DELETE FROM URPScoreHelper WHERE id='");
		strcat(query, student_id);
		strcat(query, "';");*/
		char *query = new char[strlen("UPDATE URPScoreHelper SET openid=NULL") + 128 + strlen(" WHERE id='") + 36 + 3 + 1];
		memset(query, 0, strlen("UPDATE URPScoreHelper SET openid=NULL") + 128 + strlen(" WHERE id='") + 36 + 3 + 1);
		strcpy(query, "UPDATE URPScoreHelper SET openid=NULL WHERE id='");
		strcat(query, student_id);
		strcat(query, "';");

		char **db_Result = NULL;
		sqlite3_stmt *stmt;
		db_ret = sqlite3_prepare(db, query, strlen(query), &stmt, 0);

		if (db_ret != SQLITE_OK)
		{
			cout << "Status: 500 Internal Server Error\n";
			char Err_Msg[512] = "<b>解除绑定失败，请稍后再试。</b><p>(";
			strcat(Err_Msg, sqlite3_errmsg(db));
			strcat(Err_Msg, ")</p>");
			Error(Err_Msg);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			delete[]query;
			return;
		}

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			break;
		}

		sqlite3_finalize(stmt);
		sqlite3_close(db);
		delete[]query;
		cout << "Status: 302 Found\nLocation: main.cgi\n" << GLOBAL_HEADER;
		return;
	}

	pStr1 = strstr(CGI_QUERY_STRING, "openid=");
	if (pStr1 == NULL)
	{
		cout << "Status: 500 Internal Server Error\n";
		Error("鉴权失败 (Null openid)");
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
		int m_iResult = 0;
		bool m_need_update_cookie = false;
		char *m_photo = (char *)malloc(102424);
		ZeroMemory(m_photo, 102424);
		process_cookie(&m_need_update_cookie, m_photo);

		if (strlen(m_photo) != 0) // 如果都登录了？那就踢到主页面去。
		{
			if (strcmp(CGI_SCRIPT_NAME, "/main.cgi") == 0)
			{
				parse_main(m_need_update_cookie, m_photo, false);
				delete[]openid;
				return;
			}
			cout << "Status: 302 Found\nLocation: main.cgi\n" << GLOBAL_HEADER;
			delete[]openid;
			return;
		}

		// 如果传进 sid，则自动填写学号、并且从数据库中拿密码。
		pStr1 = strstr(CGI_QUERY_STRING, "stid=");
		char stid[36] = { 0 };
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
		}

		char pass[512] = {0};
		if (strlen(stid) != 0)
		{
			sqlite3 * db = NULL;
			int db_ret = sqlite3_open("URPScoreHelper.db", &db);
			if (db_ret != SQLITE_OK)
			{
				cout << "Status: 500 Internal Server Error\n";
				Error("打开数据库文件失败，请检查 URPScoreHelper.db 是否存在。");
				delete[]openid;
				return;
			}

			// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
			char *query = new char[strlen("SELECT password FROM URPScoreHelper WHERE id='") + 128 + 1];
			memset(query, 0, strlen("SELECT password FROM URPScoreHelper WHERE id='") + 128 + 1);
			strcpy(query, "SELECT password FROM URPScoreHelper WHERE id='");
			strcat(query, stid);
			strcat(query, "';");

			char **db_Result = NULL;
			sqlite3_stmt *stmt;
			db_ret = sqlite3_prepare(db, query, strlen(query), &stmt, 0);

			if (db_ret != SQLITE_OK)
			{
				cout << "Status: 500 Internal Server Error\n";
				char Err_Msg[512] = "<b>数据库准备失败！请确认数据库合法性。</b><p>(";
				strcat(Err_Msg, sqlite3_errmsg(db));
				strcat(Err_Msg, ")</p>");
				Error(Err_Msg);
				sqlite3_finalize(stmt);
				sqlite3_close(db);
				delete[]openid;
				delete[]query;
				return;
			}

			const unsigned char *password = NULL;

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				password = sqlite3_column_text(stmt, 0);
				break;
			}
			if (password != NULL)
			{
				strcpy(pass, (const char *)password);
			}
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			delete[]query;
		}

		// 置随机数种子，并取得一个随机数，用于获取验证码。
		srand((int)time(0));
		int m_rand = rand();
		char Captcha[256] = { 0 };
		sprintf(Captcha, REQUEST_CAPTCHA, m_rand, JSESSIONID);

		// 发送验证码请求，获取验证码数据。
		char * m_rep_body = (char *)malloc(8192);
		ZeroMemory(m_rep_body, 8192);
		if (!CrawlRequest(Captcha, m_rep_body, 8192, &m_iResult))
		{
			free(m_rep_body);
			delete[]openid;
			return;
		}

		// 从返回数据流中获取验证码图片。
		char *pStr1 = strstr(m_rep_body, "\r\n\r\n");
		if (pStr1 == NULL)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法分析验证码响应协议。</p>");
			delete[]openid;
			return;
		}
		pStr1 += 4; // 指针后移四位，指向 jpg 开始位置。
		int m_CaptchaLength = m_iResult - (pStr1 - m_rep_body); // 验证码图片的大小

		// 将验证码转化为 base64 编码后的 DataURL，浏览器直接显示，供用户查看。
		char *m_base64 = new char[m_CaptchaLength * 2 + 1];
		base64_encode((const unsigned char *)pStr1, m_base64, m_CaptchaLength);
		char *m_DataURL = new char[m_CaptchaLength * 2 + 24];;
		strcpy(m_DataURL, "data:image/jpg;base64,");
		strcat(m_DataURL, m_base64);
		free(m_rep_body);

		// 读入主页面文件
		FILE *m_file_homepage = fopen(CGI_PATH_TRANSLATED, "rb");
		if (m_file_homepage == NULL)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>错误：找不到绑定页面模板。</p>");
			delete[]openid;
			return;
		}
		fseek(m_file_homepage, 0, SEEK_END); // 移到尾
		int m_file_homepage_length = ftell(m_file_homepage); // 获取文件长度
		fseek(m_file_homepage, 0, SEEK_SET); // 重新移回来
		char *m_lpszHomepage = (char *)malloc(m_file_homepage_length + 1);
		ZeroMemory(m_lpszHomepage, m_file_homepage_length + 1);
		if (fread(m_lpszHomepage, m_file_homepage_length, 1, m_file_homepage) != 1) // 将硬盘数据拷至内存
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法读取绑定页模板内容。</p>");
			fclose(m_file_homepage);
			free(m_lpszHomepage);
			delete[]openid;
			return;
		}
		fclose(m_file_homepage); // 关闭文件

		// 输出网页
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";

		cout << GLOBAL_HEADER;

		char title[256] = { 0 };
		strcat(title, "QQ用户绑定 - ");
		strcat(title, SOFTWARE_NAME);
		fprintf(stdout, header, title);

		if (strlen(stid) == 0)
		{
			fprintf(stdout, m_lpszHomepage, "感谢使用QQ登录，请先绑定自己的学号吧 :)",
				openid, "block", "", "block", pass, m_DataURL,
				SOFTWARE_NAME, __DATE__, __TIME__, CGI_SERVER_SOFTWARE);
		}
		else if(strlen(pass) == 0)
		{
			fprintf(stdout, m_lpszHomepage, "感谢使用QQ登录，请输入学号对应密码来继续操作 :)",
				openid, "none", stid, "block", pass, m_DataURL,
				SOFTWARE_NAME, __DATE__, __TIME__, CGI_SERVER_SOFTWARE);
		}
		else
		{
			fprintf(stdout, m_lpszHomepage, "感谢使用QQ登录，请输入验证码来继续操作 :)",
				openid, "none", stid, "none", pass, m_DataURL,
				SOFTWARE_NAME, __DATE__, __TIME__, CGI_SERVER_SOFTWARE);
		}
		cout << footer;

		free(m_lpszHomepage);
		delete[]m_base64;
		delete[]m_DataURL;
	}
	else // 提交账号密码验证码，打算登录绑定了
	{
		// 获取 POST 数据。
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>发生错误，POST 数据长度异常。</p>");
			delete[]openid;
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);	// TORESEARCH
		if (fgets(m_post_data, m_post_length + 1, stdin) == NULL)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>发生错误，POST 数据拉取失败。</p>");
			delete[]openid;
			return;
		}

		// 获取学号
		char *pStr1 = strstr(m_post_data, "xh=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取学号信息。</p>");
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
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取密码信息。</p>");
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
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取验证码信息。</p>");
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
		sqlite3 * db = NULL;
		int db_ret = sqlite3_open("URPScoreHelper.db", &db);
		if (db_ret != SQLITE_OK)
		{
			cout << "Status: 500 Internal Server Error\n";
			Error("打开数据库文件失败，请检查 URPScoreHelper.db 是否存在。");
			delete[]openid;
			return;
		}

		// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
		char *query = new char[strlen("UPDATE URPScoreHelper SET openid='") + 128 + strlen(" WHERE id='") + 36 + 1 + 1];
		memset(query, 0, strlen("UPDATE URPScoreHelper SET openid='") + 128 + strlen(" WHERE id='") + 36 + 1 + 1);
		strcpy(query, "UPDATE URPScoreHelper SET openid='");
		strcat(query, openid);
		strcat(query, "' WHERE id='");
		strcat(query, m_xuehao);
		strcat(query, "';");

		char **db_Result = NULL;
		sqlite3_stmt *stmt;
		db_ret = sqlite3_prepare(db, query, strlen(query), &stmt, 0);

		if (db_ret != SQLITE_OK)
		{
			cout << "Status: 500 Internal Server Error\n";
			char Err_Msg[1024] = "<b>很抱歉，QQ绑定失败。</b><p>数据库错误 (";
			strcat(Err_Msg, sqlite3_errmsg(db));
			strcat(Err_Msg, ")</p><p>但是别方吖，还可以正常登录的。</p>");
			Error(Err_Msg);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			delete[]openid;
			delete[]query;
			return;
		}

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		sqlite3_close(db);

		cout << "Status: 302 Found\n";
		cout << "Location: main.cgi\n";
		cout << GLOBAL_HEADER;
	}

	delete[]openid;
	return;
}
