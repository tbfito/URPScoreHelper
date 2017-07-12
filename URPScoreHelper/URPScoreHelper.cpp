/*
******************************************
********** iEdon-URPScoreHelper **********
**********       Ψ��С����       **********
********** CPP MVC - Controller **********
**********  Copyright (C) iEdon **********
******************************************
*/

#include "stdafx.h"
#include "URPScoreHelper.h"
#include "General.h"
#include "StringHelper.h"
#include "CCurlTask.h"
#include "OAuth2.h"
#include "URPRequests.h"
#include "INIReader.h"
#include "Encrypt.h"

// ��ں��� (FastCGI ����ѭ��)
int main(int argc, const char* argv[])
{
	/*EnableMemLeakCheck();*/
	LoadConfig();
	FCGX_Init();
	curl_global_init(CURL_GLOBAL_ALL);
	static char *emptystr = "";
	isPageSrcLoadSuccess = false;

	int FCGX_SocketId = 0;
	if (argc == 3)
	{
		if (strcmp(argv[1], "-p") == 0)
		{
			FCGX_SocketId = FCGX_OpenSocket(argv[2], 5);
			if (FCGX_SocketId == -1)
				FCGX_SocketId = 0;
		}
	}

	FCGX_InitRequest(&request, FCGX_SocketId, 0);

	while (FCGX_Accept_r(&request) == 0)
	{
		if (isdbReady)
		{
			SetUsersCounter();
		}

		fcgi_streambuf cin_fcgi_streambuf(request.in);
		fcgi_streambuf cout_fcgi_streambuf(request.out);
		fcgi_streambuf cerr_fcgi_streambuf(request.err);

		cin.rdbuf(&cin_fcgi_streambuf);
		cout.rdbuf(&cout_fcgi_streambuf);
		cerr.rdbuf(&cerr_fcgi_streambuf);

		CGI_REQUEST_URI = FCGX_GetParam("REQUEST_URI", request.envp); // ����URI
		CGI_REQUEST_METHOD = FCGX_GetParam("REQUEST_METHOD", request.envp); // ���󷽷�
		CGI_CONTENT_LENGTH = FCGX_GetParam("CONTENT_LENGTH", request.envp); // ���ݳ���
		CGI_SCRIPT_NAME = FCGX_GetParam("SCRIPT_NAME", request.envp); // �ű�����
		CGI_QUERY_STRING = FCGX_GetParam("QUERY_STRING", request.envp); // ��ѯ����
		CGI_SCRIPT_FILENAME = FCGX_GetParam("SCRIPT_FILENAME", request.envp); // �ű�λ��
		CGI_HTTP_COOKIE = FCGX_GetParam("HTTP_COOKIE", request.envp); // Cookie
		CGI_HTTPS = FCGX_GetParam("HTTPS", request.envp);
		CGI_HTTP_HOST = FCGX_GetParam("HTTP_HOST", request.envp);

		if (!isdbReady)
		{
			cout << "Status: 500 Internal Server Error\r\n"
				<< GLOBAL_HEADER
				<< "<p><b>���ݿ��ʧ��</b></p><p>���� Database.db �Ƿ���ڡ�</p>";
			goto END_REQUEST;
		}

		if (!isPageSrcLoadSuccess)
		{
			LoadPageSrc();
			if (!isPageSrcLoadSuccess)
			{
				cout << "Status: 500 Internal Server Error\r\n"
					<< GLOBAL_HEADER
					<< "<p>��ҳģ���ļ�ȱʧ���쳣��</p>";
				goto END_REQUEST;
			}
		}

		if (CGI_REQUEST_METHOD == NULL || CGI_SCRIPT_NAME == NULL || CGI_QUERY_STRING == NULL ||
			CGI_SCRIPT_FILENAME == NULL || CGI_CONTENT_LENGTH == NULL)
		{
			cout << "Status: 500 Internal Server Error\r\n"
				<< GLOBAL_HEADER
				<< "<p>FastCGI �ӿ��쳣���������á�</p>";
			goto END_REQUEST;
		}
		if (CGI_HTTP_COOKIE == NULL)
		{
			CGI_HTTP_COOKIE = emptystr;
		}

		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0) // ����� GET ����
		{
			if (strcmp(CGI_SCRIPT_NAME, "/") == 0 || strcmp(CGI_SCRIPT_NAME, "/index.fcgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=logout") == 0)
				{
					student_logout();
					cout << "Status: 302 Found\r\n" << "Location: /\r\n" << GLOBAL_HEADER;
					goto END_REQUEST;
				}
				if (strcmp(CGI_QUERY_STRING, "act=requestAssoc") == 0)
				{
					bool m_need_update_cookie = false;
					std::string photo(" ");
					process_cookie(&m_need_update_cookie, photo);
					if (photo.empty())
					{
						cout << "Status: 302 Found\r\n" << "Location: /index.fcgi\r\n" << GLOBAL_HEADER;
						goto END_REQUEST;
					}
					char student_id[512] = { 0 };
					get_student_id(student_id);
					student_logout();
					EnCodeStr(student_id, student_id);
					cout << "Status: 302 Found\r\n" << "Location: OAuth2.fcgi?stid=" << student_id << "\r\n" << GLOBAL_HEADER;
					goto END_REQUEST;
				}
				if (strcmp(CGI_REQUEST_URI, "/index.fcgi") == 0)
				{
					cout << "Status: 302 Found\r\n" << "Location: /\r\n" << GLOBAL_HEADER;
					goto END_REQUEST;
				}
				parse_index();
				goto END_REQUEST;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/main.fcgi") == 0)
			{
				bool need_update_cookie = false;
				std::string nullphoto;
				parse_main(need_update_cookie, nullphoto);
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
				if (strcmp(CGI_QUERY_STRING, "act=system_registration") == 0)
				{
					system_registration();
					goto END_REQUEST;
				}
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
			cout << "Status: 404 Not Found\r\n";
			Error("<p>�Ҳ�����ҳ�档</p>");
			goto END_REQUEST;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0) // ����� POST ����
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
				std::string nullphoto;
				parse_main(false, nullphoto);
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
		Error("<p>��������δ��������쳣��</p>");
		goto END_REQUEST;

		END_REQUEST:
			ZeroMemory(JSESSIONID, 256);
			//FCGX_Finish_r(&request); ע�� FCGI ������ÿ��ִ�� Accept���Ѿ��������� Finish ������
			//_CrtDumpMemoryLeaks();
			continue;
	}
		printf("%s\n%s\n\n%s\n", SOFTWARE_NAME, SOFTWARE_COPYRIGHT, "\tOptions: [-p (localhost):port_number]");
		curl_global_cleanup();
		sqlite3_close(db);
		free(SERVER_URL);
		free(USER_AGENT);
		free(OAUTH2_APPID);
		free(OAUTH2_SECRET);
		free(CURL_PROXY_URL);
		return 0;
}

// Ԥ����ͷ����β��ҳ��(header.fcgi, footer.fcgi, error.fcgi)
void LoadPageSrc()
{
	// ������ҳ���ļ�
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

	header = ReadTextFileToMem(file_root);

	strcpy(file_root, doc_root);
	strcat(file_root, "footer.fcgi");
	
	footer = ReadTextFileToMem(file_root);

	strcpy(file_root, doc_root);
	strcat(file_root, "error.fcgi");
	
	error = ReadTextFileToMem(file_root);
	// δ�ܼ�����Щģ��
	if (header.empty() || footer.empty() || error.empty())
	{
		isPageSrcLoadSuccess = false;
		return;
	}
	
	std::string title("��ʾ - ");
	title.append(SOFTWARE_NAME);
	ERROR_HTML = strformat(header.c_str(), title.c_str());
	ERROR_HTML += error + footer;

	delete[]doc_root;
	delete[]file_root;
	isPageSrcLoadSuccess = true;
}

// ��������
void LoadConfig()
{
	SERVER_URL = (char *)malloc(1024);
	USER_AGENT = (char *)malloc(1024);
	OAUTH2_APPID = (char *)malloc(1024);
	OAUTH2_SECRET = (char *)malloc(1024);
	CURL_PROXY_URL = (char *)malloc(1024);
	char *lpvBuffer = (char *)malloc(128);

	memset(SERVER_URL, 0, 1024);
	memset(USER_AGENT, 0, 1024);
	memset(OAUTH2_APPID, 0, 1024);
	memset(OAUTH2_SECRET, 0, 1024);
	memset(CURL_PROXY_URL, 0, 1024);
	memset(lpvBuffer, 0, 128);

	char *Dir = (char *)malloc(260);
	memset(Dir, 0, 260);
	getcwd(Dir, MAX_PATH);
	char *pStr = strstr(Dir, "\\");
	bool isUnixBasedPath = (pStr == NULL);
	if (isUnixBasedPath) // Unix-like system
	{
		strcat(Dir, "/config.ini");
	}
	else // for windows
	{
		strcat(Dir, "\\config.ini");
	}

	INIReader reader(Dir);
	if (reader.ParseError() != 0) {
		strcpy(SERVER_URL, "http://127.0.0.1");
		strcpy(USER_AGENT, SOFTWARE_NAME);
		strcpy(lpvBuffer, "3");
		strcpy(OAUTH2_APPID, "NULL");
		strcpy(OAUTH2_SECRET, "NULL");
		strcpy(CURL_PROXY_URL, "");
	}
	else
	{
		strcpy(SERVER_URL, reader.Get("Config", "SERVER_URL", "http://127.0.0.1").c_str());
		strcpy(USER_AGENT, reader.Get("Config", "USER_AGENT", SOFTWARE_NAME).c_str());
		strcpy(lpvBuffer, reader.Get("Config", "CURL_TIMEOUT", "3").c_str());
		strcpy(OAUTH2_APPID, reader.Get("Config", "OAUTH2_APPID", "NULL").c_str());
		strcpy(OAUTH2_SECRET, reader.Get("Config", "OAUTH2_SECRET", "NULL").c_str());
		strcpy(CURL_PROXY_URL, reader.Get("Config", "CURL_PROXY_URL", "").c_str());
	}
		
	CURL_TIMEOUT = atoi(lpvBuffer);
	if (CURL_TIMEOUT <= 0)
		CURL_TIMEOUT = 3;
	memset(lpvBuffer, 0, 128);
	if (reader.ParseError() == 0)
	{
		strcpy(lpvBuffer, reader.Get("Config", "CURL_USE_PROXY", "0").c_str());
	}
	CURL_USE_PROXY = (atoi(lpvBuffer) == 1);

	free(lpvBuffer);
	free(Dir);
	
	db = NULL;
	int db_ret = sqlite3_open("Database.db", &db);
	if (db_ret != SQLITE_OK)
	{
		isdbReady = false;
	}
	else
	{
		isdbReady = true;
	}
	std::string query("CREATE TABLE IF NOT EXISTS \"URPScoreHelper\" (\"id\" TEXT(36) NOT NULL, \"password\" TEXT(36) NOT NULL, \"openid\" TEXT(128), \"lastlogin\" TEXT(64), PRIMARY KEY (\"id\" ASC));");
	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);
	if (db_ret != SQLITE_OK)
	{
		printf( "fuck!" );
		sqlite3_finalize(stmt);
		return;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		break;
	}
	sqlite3_finalize(stmt);
}

// �����û�����������
void SetUsersCounter()
{
	// ��ȡ�����û�ʹ�������ǵķ��� :)
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
	if (g_fQueryCount != NULL)
	{
		fseek(g_fQueryCount, 0, SEEK_SET);
		fclose(g_fQueryCount);
	}

	std::string query("SELECT COUNT(*) FROM URPScoreHelper;");

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		sqlite3_finalize(stmt);
		return;
	}

	const unsigned char *counts = NULL;
	g_users = 0;
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		counts = sqlite3_column_text(stmt, 0);
		break;
	}
	if (counts != NULL)
	{
		g_users = atoi((const char *)counts);
	}
	sqlite3_finalize(stmt);
}

// ���� Cookie����Ƭ(p_photo_uri Ϊ�մ���Ҫ��Ƭ, ����������ݲ�Ϊ�����������д����Ƭ����)
int process_cookie(bool *p_need_update_cookie, std::string & p_photo_uri)
{
	if (strcmp(CGI_HTTP_COOKIE, "") != 0) // ����ͻ����Ѿ��õ� JSESSIONID������ԭ Cookie �Ƿ���ڡ���Ч�����������Ƿ��������� Cookie��
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
				Error("<p>�޷���ȡ Session ID</p><p>Cookie ��βʧ��</p>");
				p_photo_uri.erase();
				return -1;
			}
			mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // �ɹ������ Session ID��
			*p_need_update_cookie = true;
			return -1;
		}
		else // ��� Cookie �����ã��ͻ�ȡ����
		{
			char *pStr1 = strstr(CGI_HTTP_COOKIE, "JSESSIONID=");
			if (pStr1 != NULL)
			{
				char *pStr2 = strstr(pStr1 + 11, ";");
				if (pStr2 == NULL) // ������� Cookie �����һ��
				{
					right(JSESSIONID, CGI_HTTP_COOKIE, strlen(CGI_HTTP_COOKIE) - (pStr1 - CGI_HTTP_COOKIE) - 11);
				}
				else
				{
					mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11);
				}
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
		// ��ȡ Session ID��
		char *pStr1 = strstr(m_rep_header, "JSESSIONID=");
		if (pStr1 == NULL)
		{
			Error("<p>�޷���ȡ Session ID</p><p>Cookie ��ͷʧ��</p>");
			p_photo_uri.erase();
			return -1;
		}
		char *pStr2 = strstr(pStr1 + 11, ";");
		if (pStr2 == NULL)
		{
			Error("<p>�޷���ȡ Session ID</p><p>Cookie ��βʧ��</p>");
			p_photo_uri.erase();
			return -1;
		}

		mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // �ɹ���� Session ID��
		*p_need_update_cookie = true;
	}

	if (p_photo_uri.empty())
		return -1;  // p_photo_uri ָ���� NULL ������Ҫ��Ƭ��

	// ������¼û
	std::string Jsess ("JSESSIONID=");
	Jsess += JSESSIONID;

	CCurlTask req;
	if (!req.Exec(false, REQUEST_PHOTO, Jsess))
	{
		p_photo_uri.erase();
		return -1;
	}
	char *m_photo = req.GetResult();

	if (strstr(m_photo, "��¼") == NULL)
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

// ���� GET /main.fcgi
int parse_main(bool p_need_set_cookie, std::string & p_photo)
{
	if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
	{
		// ��ȡ POST ���ݡ�
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0 || m_post_length > 127)
		{
			Error("<p><b>��������POST ���ݳ����쳣</b></p><p>�ʺŻ���������������Ŷ��������</p>");
			return -1;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

		// ��ȡѧ��
		char *pStr1 = strstr(m_post_data, "xh=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error("<p>�޷���ȡѧ����Ϣ��</p>");
			return -1;
		}
		char *pStr2 = strstr(pStr1 + 3, "&");
		char m_xuehao[128] = { 0 };
		mid(m_xuehao, pStr1, pStr2 - pStr1 - 3, 3);
		pStr1 = NULL;
		pStr2 = NULL;

		// ��ȡ����
		pStr1 = strstr(m_post_data, "mm=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error("<p>�޷���ȡ������Ϣ��</p>");
			return -1;
		}
		pStr2 = strstr(pStr1 + 3, "&");
		char m_password[128] = { 0 };
		mid(m_password, pStr1, pStr2 - pStr1 - 3, 3);
		pStr1 = NULL;
		pStr2 = NULL;

		// ��ȡ��֤��
		pStr1 = strstr(m_post_data, "yzm=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error("<p>�޷���ȡ��֤����Ϣ��</p>");
			return -1;
		}
		char m_captcha[128] = { 0 };
		right(m_captcha, pStr1 + 4, 4);

		if (!student_login(m_xuehao, m_password, m_captcha))
		{
			// ������Դ��������ѧ����¼���������ˡ�
			free(m_post_data);
			return -1;
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
	if (p_photo.empty())
	{
		p_photo = " "; // �� p_photo �����ݣ����� process_cookie �����Ƭ��ȡ�ܽ��С�
		int ret = process_cookie(&p_need_set_cookie, p_photo);
		if (p_photo.empty())
		{
			cout << "Status: 302 Found\r\nLocation: index.fcgi\r\n" << GLOBAL_HEADER;
			return -1;
		}
	}
	// ������ҳ���ļ�
	std::string m_lpszHomepage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	char m_student_name[512] = {0};
	char m_student_id[512] = { 0 };
	get_student_name(m_student_name);
	get_student_id(m_student_id);

	// �����ҳ
	if (p_need_set_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";

	cout << GLOBAL_HEADER;

	std::string title(m_student_name);
	title += " - ";
	title += SOFTWARE_NAME;

	// SQLite3 ���ݿ⣬���� main���� URLScoreHelper���ֶ� text id(36), text password(36), text openid(128)��
	std::string query("SELECT openid FROM URPScoreHelper WHERE id='");
	query.append(m_student_id);
	query.append("';");

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		std::string Err_Msg("<b>���ݿ�׼��ʧ�ܣ���ȷ�����ݿ�Ϸ��ԡ�</b><p>(");
		Err_Msg.append(sqlite3_errmsg(db));
		Err_Msg.append(")</p>");
		Error(Err_Msg.c_str());
		sqlite3_finalize(stmt);
		return -1;
	}

	const unsigned char *openid = NULL;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		openid = sqlite3_column_text(stmt, 0);
		break;
	}

	sqlite3_finalize(stmt);

	cout << strformat(header.c_str(), title.c_str());

	if (openid == NULL)
	{
		cout << strformat( m_lpszHomepage.c_str(), p_photo.c_str(), m_student_name, m_student_id,
			"inline-block", "none", "");
	}
	else
	{
		cout << strformat( m_lpszHomepage.c_str(), p_photo.c_str(), m_student_name, m_student_id,
			"none", "inline-block", m_student_id);
	}

	cout << footer.c_str();
	return 0;
}

// ������ҳ������ (GET / /index.fcgi)
int parse_index()
{
	// �����QQ��¼���������Զ�����ʺ����롣
	char *m_xh = NULL;
	char *m_mm = NULL;
	char *token_xh = NULL;
	char *token_mm = NULL;
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
			if (pStr2 == NULL) // ������� Cookie �����һ��
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

	// ������ҳ���ļ�
	std::string m_lpszHomepage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	cout << GLOBAL_HEADER;
	cout << strformat( header.c_str(), SOFTWARE_NAME);

	if (m_xh == NULL || m_mm == NULL)
	{
		if (token_xh != NULL && token_mm != NULL)
		{
			cout << strformat(m_lpszHomepage.c_str(), SOFTWARE_NAME, g_users, g_QueryCount,
				"������Ľ���ϵͳ�ʺ�����¼�� :)", "flex", token_xh, "flex", token_mm, "block", "block", "none");
		}
		else
		{
			cout << strformat(m_lpszHomepage.c_str(), SOFTWARE_NAME, g_users, g_QueryCount,
				"������Ľ���ϵͳ�ʺ�����¼�� :)", "flex", "", "flex", "", "block", "block", "none");
		}
	}
	else 
	{
		cout << strformat( m_lpszHomepage.c_str(), SOFTWARE_NAME, g_users, g_QueryCount,
						"QQ��¼�ɹ���������֤������� :)", "none", m_xh, "none", m_mm, "none", "none", "block");
	}

	cout << footer.c_str();
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

// ������֤�� Ajax ����
void parse_ajax_captcha() //(AJAX: GET /captcha.fcgi)
{
	cout << "Cache-Control: no-cache\r\nPragma: no-cache\r\nExpires: -1\r\n";
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // �����ݣ���Ҫ��ȡ��Ƭ
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
	
	if (!m_photo.empty() && !m_need_update_cookie) // ��¼�˾�ͨ���Ѿ���¼
	{
		cout << "Content-Type: text/plain; charset=gb2312\r\n\r\nLOGGED-IN";
		return;
	}

	// ����������ӣ���ȡ��һ������������ڻ�ȡ��֤�롣
	srand((int)time(0));
	int m_rand = rand();
	char Captcha[256] = { 0 };
	sprintf(Captcha, REQUEST_CAPTCHA, m_rand);

	// ������֤�����󣬻�ȡ��֤�����ݡ�
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
		cout << "Content-Type: text/plain; charset=gb2312\r\n\r\nREQUEST-FAILED";
		return;
	}
	char *m_rep_body = req.GetResult();

	int m_CaptchaLength = req.GetLength(); // ��֤��ͼƬ�Ĵ�С

	// ����֤��ת��Ϊ base64 ������ DataURL�������ֱ����ʾ�����û��鿴��
	char *m_base64 = new char[m_CaptchaLength * 2 + 1];
	base64_encode((const unsigned char *)m_rep_body, m_base64, m_CaptchaLength);
	char *m_DataURL = new char[m_CaptchaLength * 2 + 24];;
	strcpy(m_DataURL, "data:image/jpg;base64,");
	strcat(m_DataURL, m_base64);

	cout << "Content-Type: text/plain\r\n\r\n";
	cout << m_DataURL;

	delete[]m_base64;
	delete[]m_DataURL;
}

// �����ѯҳ������ (GET /query.fcgi)
int parse_query()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // �����ݣ���Ҫ��ȡ��Ƭ
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // ��û��½�Ͷ�ȥ��½��
	{
		cout << "Status: 302 Found\r\nLocation: index.fcgi\r\n" << GLOBAL_HEADER;
		return 0;
	}

	// ��ʼ���(��ѧ��)��
	CCurlTask req;
	if (!req.Exec(false, REQUEST_QUERY_SCORE, CGI_HTTP_COOKIE))
	{
		Error("<p><b>��������ʧ��</b></p><p>curl ����ʧ��</p>");
		student_logout();
		return -1;
	}
	char *m_rep_body = req.GetResult();

	// �Ż����ܽ������ʾ��ѯҳ��
	string result(m_rep_body);
	parse_friendly_score(result);
	return 0;
}

// �������ҳ��
void parse_friendly_score(std::string & p_strlpszScore)
{
	char m_Student[64] = { 0 };
	get_student_name(m_Student);

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	char *p_lpszScore = (char *)malloc(p_strlpszScore.length() + 1);
	strcpy(p_lpszScore, p_strlpszScore.c_str());

	char *m_query_not_reg = strstr(p_lpszScore, "û��ע��");
	if (m_query_not_reg != NULL)
	{
		std::string m_original_str ("<p><b>�װ���%s������ѧ�ڻ�û�е���ע��</b></p><p>��ע��Ļ����ǲ鲻����Ϣ��Ŷ��</p><p>�ҿ���ʩչ������\
<b>һ�������ڽ���ϵͳע��Ŷ~</b></p><p>--&gt; �㰴�·���ť���Զ�ע�ᣬֱ���ֽ��� :P &lt;--</p>\
<div class=\"weui-msg__opr-area\"><p class=\"weui-btn-area\"><a style=\"color:#fff\" href=\"query.fcgi?act=system_registration\" class=\"weui-btn weui-btn_primary\">һ��ע��</a></p></div>");
		m_original_str = strformat(m_original_str.c_str(), m_Student);
		Error(m_original_str.c_str());
		return;
	}
	if (strcmp(CGI_QUERY_STRING, "order=tests") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, GET_SMALL_TEST_SCORE, CGI_HTTP_COOKIE))
		{
			Error("<p><b>��������ʧ��</b></p><p>curl ����ʧ��</p>");
			return;
		}
		char *m_rep_body = req.GetResult();
		char *m_result = strstr(m_rep_body, "<table cellpadding=\"0\" width=\"100%\" class=\"displayTag\" cellspacing=\"1\" border=\"0\" id=\"user\">");
		if (m_result == NULL)
		{
			Error("<p><b>�ӷ�������ȡ����ʧ�ܡ�(BeginOfTable)</b></p><p>������������߷�æ��666 ���Ժ����ԡ�</p><p>���������æ�������������ʾ��©����ˢ�¼��μ��ɡ�</p>");
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
			Error("<p>�ӷ�������ȡ����ʧ�ܡ�(EndOfBodyNotFound)</p>");
			return;
		}
		m_result -= 93;
		cout << GLOBAL_HEADER;
		char m_before[512] = { 0 };
		sprintf(m_before, "<a name=\"qb_731\"></a><table width=\"100%%\" border=\"0\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td class=\"Linetop\"></td></tr></tbody></table><table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\"><tbody><tr><td width=\"100%%\"><table border=\"0\" align=\"left\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td>&nbsp;</td><td valign=\"middle\">&nbsp;<b>%s</b> &nbsp;</td></tr></tbody></table></td></tr></tbody></table>", "�ɼ��嵥���¿�/����/����/����/�忼��");
		*(m_end_body + 8) = '<';
		*(m_end_body + 9) = '/';
		*(m_end_body + 10) = 'd';
		*(m_end_body + 11) = 'i';
		*(m_end_body + 12) = 'v';
		*(m_end_body + 13) = '>';
		*(m_end_body + 14) = '\0';

		strcat(m_prep, m_before);
		strcat(m_prep, m_result);

		std::string title(m_Student);
		title += "�Ŀ��Գɼ� - ";
		title += SOFTWARE_NAME;

		cout << strformat( header.c_str(), title.c_str());
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		cout << footer.c_str();

		g_fQueryCount = fopen("QueryCount.bin", "w+");
		if (g_fQueryCount != NULL)
		{
			fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
			fclose(g_fQueryCount);
		}

		free(m_prep);
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=passed") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_QBINFO, CGI_HTTP_COOKIE))
		{
			Error("<p><b>��������ʧ��</b></p><p>curl ����ʧ��</p>");
			return;
		}
		char *m_rep_body = req.GetResult();
		char *m_result = strstr(m_rep_body, "<body leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\" style=\"overflow:auto;\">");
		if (m_result == NULL)
		{
			Error("<p><b>�ӷ�������ȡ����ʧ�ܡ�(BeginOfRet)</b></p><p>������������߷�æ��666 ���Ժ����ԡ�</p><p>���������æ�������������ʾ��©����ˢ�¼��μ��ɡ�</p>");
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
			Error("<p>�ӷ�������ȡ����ʧ�ܡ�(EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		std::string title(m_Student);
		title += "��ͨ����Ŀ - ";
		title += SOFTWARE_NAME;

		cout << strformat( header.c_str(), title.c_str());
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		cout << footer.c_str();

		g_fQueryCount = fopen("QueryCount.bin", "w+");
		if (g_fQueryCount != NULL)
		{
			fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
			fclose(g_fQueryCount);
		}

		free(m_prep);
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=byplan") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_PLAN, CGI_HTTP_COOKIE))
		{
			Error("<p><b>��������ʧ��</b></p><p>curl ����ʧ��</p>");
			return;
		}
		char *m_rep_body = req.GetResult();
		char *m_result = strstr(m_rep_body, "<body leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\" style=\"overflow:auto;\">");
		if (m_result == NULL)
		{
			Error("<p><b>�ӷ�������ȡ����ʧ�ܡ�(BeginOfRet)</b></p><p>������������߷�æ��666 ���Ժ����ԡ�</p><p>���������æ�������������ʾ��©����ˢ�¼��μ��ɡ�</p>");
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
			Error("<p>�ӷ�������ȡ����ʧ�ܡ�(EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		std::string title(m_Student);
		title += "��רҵ���� - ";
		title += SOFTWARE_NAME;

		cout << strformat( header.c_str(), title.c_str());
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		cout << footer.c_str();
		g_fQueryCount = fopen("QueryCount.bin", "w+");
		if (g_fQueryCount != NULL)
		{
			fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
			fclose(g_fQueryCount);
		}

		free(m_prep);
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=failed") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, GET_GRADE_BY_FAILED, CGI_HTTP_COOKIE))
		{
			Error("<p><b>��������ʧ��</b></p><p>curl ����ʧ��</p>");
			return;
		}
		char *m_rep_body = req.GetResult();
		char *m_result = strstr(m_rep_body, "<table width=\"100%\"  border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\">");
		if (m_result == NULL)
		{
			Error("<p><b>�ӷ�������ȡ����ʧ�ܡ�(BeginOfRet)</b></p><p>������������߷�æ��666 ���Ժ����ԡ�</p><p>���������æ�������������ʾ��©����ˢ�¼��μ��ɡ�</p>");
			return;
		}
		m_result = strstr(m_result + 92, "<table width=\"100%\"  border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\">");
		if (m_result == NULL)
		{
			Error("<p><b>�ӷ�������ȡ����ʧ�ܡ�(MidOfRet-Table)</b></p><p>������������߷�æ��666 ���Ժ����ԡ�</p><p>���������æ�������������ʾ��©����ˢ�¼��μ��ɡ�</p>");
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
			Error("<p>�ӷ�������ȡ����ʧ�ܡ�(EndOfBodyNotFound)</p>");
			return;
		}
		cout << GLOBAL_HEADER;
		*(m_end_body + 2) = 'd';
		*(m_end_body + 3) = 'i';
		*(m_end_body + 4) = 'v';
		*(m_end_body + 5) = '>';
		*(m_end_body + 6) = '\0';
		strcat(m_prep, m_result);

		std::string title(m_Student);
		title += "��δͨ����Ŀ - ";
		title += SOFTWARE_NAME;

		cout << strformat( header.c_str(), title.c_str());
		cout << strformat( m_lpszQuery.c_str(), m_Student, m_prep);
		cout << footer.c_str();

		g_fQueryCount = fopen("QueryCount.bin", "w+");
		if (g_fQueryCount != NULL)
		{
			fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
			fclose(g_fQueryCount);
		}

		free(m_prep);
		return;
	}

	if (strcmp(CGI_QUERY_STRING, "order=schedule") == 0)
	{
		free(p_lpszScore);
		CCurlTask req;
		if (!req.Exec(false, REQUEST_SCHEDULE, CGI_HTTP_COOKIE))
		{
			Error("<p><b>��������ʧ��</b></p><p>curl ����ʧ��</p>");
			return;
		}
		char *m_rep_body = req.GetResult();
		char *m_result = strstr(m_rep_body, "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"titleTop2\">");
		if (m_result == NULL)
		{
			Error("<p><b>�ӷ�������ȡ�γ̱�ʧ�ܡ�(BeginOfTable)</b></p><p>�������Ժ�����</p>");
			return;
		}
		m_result += 81;
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
			Error("<p>�ӷ�������ȡ�γ̱�ʧ�ܡ�(EndOfBodyNotFound)</p>");
			return;
		}
		m_result -= 81;
		cout << GLOBAL_HEADER;
		char m_before[512] = { 0 };
		sprintf(m_before, "<a name=\"qb_731\"></a><table width=\"100%%\" border=\"0\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td class=\"Linetop\"></td></tr></tbody></table><table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"title\" id=\"tblHead\"><tbody><tr><td width=\"100%%\"><table border=\"0\" align=\"left\" cellpadding=\"0\" cellspacing=\"0\"><tbody><tr><td>&nbsp;</td><td valign=\"middle\">&nbsp;<b>%s</b> &nbsp;</td></tr></tbody></table></td></tr></tbody></table>", "�ҵĿγ̱� / ѡ�ν��");
		*(m_end_body + 8) = '<';
		*(m_end_body + 9) = '/';
		*(m_end_body + 10) = 'd';
		*(m_end_body + 11) = 'i';
		*(m_end_body + 12) = 'v';
		*(m_end_body + 13) = '>';
		*(m_end_body + 14) = '\0';

		strcat(m_prep, m_before);
		strcat(m_prep, m_result);

		std::string title("��ѧ�ڿγ̱� - ");
		title += SOFTWARE_NAME;

		cout << strformat(header.c_str(), title.c_str());
		cout << strformat(m_lpszQuery.c_str(), m_Student, m_prep);
		cout << footer.c_str();

		g_fQueryCount = fopen("QueryCount.bin", "w+");
		if (g_fQueryCount != NULL)
		{
			fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
			fclose(g_fQueryCount);
		}
		free(m_prep);
		return;
	}

	// ��λ����һ��ɼ�
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

	// ѭ����ȡÿһ��ɼ���Ϣ

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

		pStr2 = pStr3;
		pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subXuefen[128] = { 0 };
		mid(m_subXuefen, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_subXuefen) == 0) sprintf(m_subXuefen, "��������");

		pStr2 = pStr3;
		pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuigaofen[128] = { 0 };
		mid(m_subzuigaofen, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_subzuigaofen) == 0) sprintf(m_subzuigaofen, "��������");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuidifen[128] = { 0 };
		mid(m_subzuidifen, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_subzuidifen) == 0) sprintf(m_subzuidifen, "��������");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subjunfen[128] = { 0 };
		mid(m_subjunfen, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atof(m_subjunfen) == 0) sprintf(m_subjunfen, "��������");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subchengji[256] = { 0 };
		mid(m_subchengji, pStr2, pStr3 - pStr2 - 19, 19);
		if (strstr(m_subchengji, "����") != NULL)
		{
			strcpy(m_subchengji,"95");
		}
		if (strstr(m_subchengji, "����") != NULL)
		{
			strcpy(m_subchengji, "85");
		}
		if (strstr(m_subchengji, "�е�") != NULL)
		{
			strcpy(m_subchengji, "75");
		}
		if (strstr(m_subchengji, "����") != NULL)
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
		if (strstr(m_subchengji, "������") != NULL)
		{
				strcpy(m_subchengji, "55");
				isPassed = false;
		}
		//if (atoi(m_subchengji) == 0) strcpy(m_subchengji, "��������");
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
		//if (atof(m_submingci) == 0) sprintf(m_submingci, "��������");

		// ������xѧ�֣�ȫ��������/��ѧ�� = ��Ȩ�֣��ų������Ϳγ����
		float m_xuefen = atof(m_subXuefen);
		float m_chengji = atof(m_subchengji);
		float m_kcxfjd = m_xuefen * cj2jd(m_chengji);
		if (strstr(m_subName, "����") == NULL && strstr(m_subName, "����ѵ��") == NULL 
			&& strstr(m_subName, "ʵ��") == NULL)
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

		char *m_StrTmp = new char[strlen(SCORE_TEMPLATE) + 50 + +strlen(m_subName) + strlen(m_subchengji) + strlen(m_subjunfen) + strlen(m_subzuigaofen) + strlen(m_subzuidifen) + strlen(m_submingci) + strlen(m_subXuefen) + 16 + 1];
		sprintf(m_StrTmp, SCORE_TEMPLATE, isPassed ? "": "background-color: rgba(255, 0, 0, 0.5);color:#fff", m_subName, m_subchengji, m_subjunfen, m_subzuigaofen, m_subzuidifen,
			m_submingci, m_subXuefen, m_kcxfjd);
		m_Output.append(m_StrTmp);
		delete[]m_StrTmp;
		m_success = true; // �鵽һ����һ��
		pStr1 = strstr(pStr3, "<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
	}
	
	// ���緢���˴���
	if (!m_success) 
	{
		free(p_lpszScore);
		Error("<p>���ã���ѯʱ���������������</p>");
		return;
	}
	if (hasChengji == false)
	{
		char *m_StrTmp = new char[strlen(SCORE_TEMPLATE) + 17 + 64 + 1];
		sprintf(m_StrTmp, SCORE_TEMPLATE, "", "��ѧ�ڻ�δ���ɼ�", "", "", "", "","", "", "");
		m_Output.append(m_StrTmp);
		delete[]m_StrTmp;
	}
	m_Output.append(AFTER_TEMPLATE);

	// ��䷵��ҳ��
	if (m_Total_pointsxxuefen != 0 || m_Total_xuefen != 0)
	{
		char m_jiaquanfen[512] = { 0 };
		sprintf(m_jiaquanfen, "<div id=\"i_total\"><p>��Ȩƽ���� / GPA(ƽ������)��</p><center>%.1f&nbsp;&nbsp;&nbsp;&nbsp;%.2f</center></div>",
				m_Total_pointsxxuefen / m_Total_xuefen, m_Total_jidian / m_Total_xuefen);
		m_Output.insert(0, m_jiaquanfen);
	}

	cout << GLOBAL_HEADER;

	std::string title(m_Student);
	title += "�ı�ѧ�ڳɼ� - ";
	title += SOFTWARE_NAME;

	cout << strformat( header.c_str(), title.c_str());
	cout << strformat( m_lpszQuery.c_str(), m_Student, m_Output.c_str());

	cout << footer.c_str();

	g_fQueryCount = fopen("QueryCount.bin", "w+");
	if (g_fQueryCount != NULL)
	{
		fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
		fclose(g_fQueryCount);
	}
	free(p_lpszScore);
}

// ��ȡѧ������
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
	// ��ȡѧ������
	char *pStr1 = strstr(m_rep_header, "<td class=\"fieldName\">����:&nbsp;</td>");
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
}

// ��ȡѧ���ʺ�
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
	// ��ȡѧ������
	char *pStr1 = strstr(m_rep_header, "��ǰ�û�:");
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

// ����ϵͳ����ע�� (GET /query.fcgi?act=system_registration)
int system_registration()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		cout << "Status: 302 Found\r\n" << "Location: index.fcgi\r\n" << GLOBAL_HEADER;
		return -1;
	}

	// �������󣬻�ȡ����ע����Ϣ��
	CCurlTask req;
	if (!req.Exec(false, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE))
	{
		Error("<p><b>Ͷ�ݵ���ע����Ϣʧ��</b></p><p>curl ����ʧ��</p>");
		return -1;
	}
	char *m_rep_header = req.GetResult();
	// ������Ҫ����ע���ѧ����Ϣ��
	char *pStr1 = strstr(m_rep_header, "selected>");
	if (pStr1 == NULL)
	{
		Error("<p>���ݴ��󡣲�����˼���Զ�ע��ʧ�ܣ��������ȥ����ϵͳ������~ (1)</p>");
		return -1;
	}
	pStr1 -= 70;
	char *pStr2 = strstr(pStr1, "<option value=\"");
	if (pStr2 == NULL)
	{
		Error("<p>���ݴ��󡣲�����˼���Զ�ע��ʧ�ܣ��������ȥ����ϵͳ������~ (2)</p>");
		return -1;
	}
	pStr1 = pStr2;
	pStr2 = strstr(pStr1 + 16, "\"");
	if (pStr2 == NULL)
	{
		Error("<p>���ݴ��󡣲�����˼���Զ�ע��ʧ�ܣ��������ȥ����ϵͳ������~ (3)</p>");
		return -1;
	}

	char m_regval[4096] = { 0 };
	mid(m_regval, pStr1, pStr2 - pStr1 - 15, 15);

	// ������ע����Ϣ
	char m_post_reg_info[4096] = "zxjxjhh=";
	strcat(m_post_reg_info, m_regval);
	int m_post_reg_info_length = strlen(m_post_reg_info);

	// ���ע������
	char m_post_req[8192] = { 0 };
	sprintf(m_post_req, REQUEST_POST_REGISTER_INTERFACE, m_regval);

	CCurlTask req2;
	// ��ʼ����ע��
	if (!req2.Exec(false, m_post_req, CGI_HTTP_COOKIE, true, m_post_reg_info))
	{
		Error("<p><b>POST����ע����Ϣʧ��</b></p><p>curl ����ʧ��</p>");
		return -1;
	}
	m_rep_header = req2.GetResult();
	// ����Ƿ�ע��ɹ���
	pStr1 = strstr(m_rep_header, "ע��ɹ�");
	if (pStr1 == NULL)
	{
		Error("<p>������˼���Զ�ע��ʧ�ܣ��������ȥ����ϵͳ������~ (4)</p>");
		return -1;
	}

	cout << "Status: 302 Found\r\n" << "Location: query.fcgi\r\n" << GLOBAL_HEADER;
	return 0;
}

// ��¼ѧ��
bool student_login(char *p_xuehao, char *p_password, char *p_captcha)
{
	// ���͵�½����
	char *m_origin = "zjh1=&tips=&lx=&evalue=&eflag=&fs=&dzslh=&zjh=%s&mm=%s&v_yzm=%s";
	char m_padding[512] = { 0 };
	sprintf(m_padding, m_origin, p_xuehao, p_password, p_captcha);

	CCurlTask req;
	if (!req.Exec(false, REQUEST_LOGIN, CGI_HTTP_COOKIE, true, m_padding))
	{
		Error("<p><b>��¼ʧ��</b></p><p>curl ����ʧ��</p>");
		return false;
	}
	char *m_rep_body = req.GetResult();

	// ��ȡ��¼�����
	char *m_result = m_rep_body;

	// �����¼�����
	char *m_login_not_auth = strstr(m_result, "֤����"); // for some urp systems
	if (m_login_not_auth != NULL)
	{
		Error("<p><b>ѧ�Ż����벻�԰�</b></p><p>��������޸Ĺ�����ϵͳ���ʺ����룬��ʹ������������һ�ԡ�</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "���벻��ȷ");
	if (m_login_not_auth != NULL)
	{
		Error("<p><b>ѧ�Ż����벻�԰���</b></p><p>��������޸Ĺ�����ϵͳ���ʺ����룬��ʹ������������һ�ԡ�</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "��֤�����");
	if (m_login_not_auth != NULL)
	{
		Error("<p>��֤�벻�ԣ�������</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "���ݿ�");
	if (m_login_not_auth != NULL)
	{
		Error("<p>ѧԺϵͳ��˵���ݿⷱæ</p><p>���ȵȵ�������~</p>");
		return false;
	}
	char *m_login_success = strstr(m_result, "ѧ�����ۺϽ���");
	if (m_login_success == NULL)
	{
		Error("<p>���š���������һ������⣡����ϵͳ������</p><p>�������Ժ������԰ɡ�</p>");
		return false;
	}

	// SQLite3 ���ݿ⣬���� main���� URLScoreHelper���ֶ� text id(36), text password(36), text openid(128)��
	std::string query("SELECT id FROM URPScoreHelper WHERE id='");
	query += p_xuehao;
	query += "';";

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		student_logout();
		std::string Err_Msg = "<b>���ݿ�׼��ʧ�ܣ���ȷ�����ݿ�Ϸ��ԡ�</b><p>(";
		Err_Msg += sqlite3_errmsg(db);
		Err_Msg += ")</p>";
		Error(Err_Msg.c_str());
		sqlite3_finalize(stmt);
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

	// ��������URL����
	int len = url_decode(p_password, strlen(p_password));
	char temp[128] = { 0 };
	left(temp, p_password, len);
	strcpy(p_password, temp);

	if (id == NULL) // �޼�¼����д�����ݿ�
	{
		std::string query("INSERT INTO URPScoreHelper (id, password, openid, lastlogin) VALUES ('");
		query += p_xuehao;
		query += "', '";
		query += p_password;
		query += "', NULL, '";
		char m_time[128] = { 0 };
		get_time(m_time);
		query += m_time;
		query += "');";

		char **db_Result = NULL;
		sqlite3_stmt *stmt;
		db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

		if (db_ret != SQLITE_OK)
		{
			student_logout();
			std::string Err_Msg("<b>�ܱ�Ǹ����¼ʧ�ܡ�</b><p>���ݿ���� (");
			Err_Msg += sqlite3_errmsg(db);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			sqlite3_finalize(stmt);
			return false;
		}
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	else // Ϊ�ɹ���¼��ѧ�����¼�¼
	{
		// SQLite3 ���ݿ⣬���� main���� URLScoreHelper���ֶ� text id(36), text password(36), text openid(128) text lastlogin(64)��
		std::string query("UPDATE URPScoreHelper SET password='");
		char m_time[128] = { 0 };
		get_time(m_time);
		query += p_password;
		query += "', lastlogin='";
		query += m_time;
		query += "' WHERE id = '";
		query += p_xuehao;
		query += "';";

		char **db_Result = NULL;
		sqlite3_stmt *stmt;
		db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

		if (db_ret != SQLITE_OK)
		{
			student_logout();
			std::string Err_Msg("<b>��¼���ݿ��¼ʧ�ܣ����Ժ����ԡ�</b><p>(");
			Err_Msg += sqlite3_errmsg(db);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			sqlite3_finalize(stmt);
			return false;
		}
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			break;
		}
		sqlite3_finalize(stmt);
	}
	// ���ˣ�ѧ����¼�ɹ�
	return true;
}

// �ǳ�ѧ��
void student_logout()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
		return;
	CCurlTask req;
	req.Exec(true, REQUEST_LOGOUT, CGI_HTTP_COOKIE);
}

// ���ܲ�ѯ��� (/QuickQuery.fcgi)
void parse_QuickQuery_Intro()
{
	bool m_need_update_cookie = false;
	std::string nullphoto;
	process_cookie(&m_need_update_cookie, nullphoto);

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
	cout << GLOBAL_HEADER;

	std::string title("���ܳɼ���ѯ - ");
	title += SOFTWARE_NAME;

	cout << strformat( header.c_str(), title.c_str());
	cout << strformat( m_lpszQuery.c_str(), SOFTWARE_NAME, g_users, g_QueryCount);
	cout << footer.c_str();
}

// ���ܲ�ѯ��� (/query.fcgi?act=QuickQuery)
void parse_QuickQuery_Result()
{
	bool m_need_update_cookie = false;
	std::string nullphoto;
	process_cookie(&m_need_update_cookie, nullphoto);

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	// ��ȡ POST ���ݡ�
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
		Error("<p>��������POST ���ݳ����쳣��</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);	// TORESEARCH
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

	// ��ȡѧ��
	char *pStr1 = strstr(m_post_data, "xh=");
	if (pStr1 == NULL)
	{
		free(m_post_data);
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
		Error("<p>�޷���ȡѧ����Ϣ��</p>");
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
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
		Error("<p>�����ѧ�Ÿ����������⣬��ȷ�ϣ�</p>");
		return;
	}
	
	std::string m_list;
	char m_xxmz[128] = { 0 };
	free(m_post_data);

	for (int xh_index = 0; xh_index < m_xhgs; xh_index++)
		{
			if (strlen(m_xh[xh_index]) != 9)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error("<p>�����ѧ�����г��ȴ������⣬��ȷ�ϣ�</p>");
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
				Error("<p>Ͷ�����ܲ�ѯ����ʧ�ܡ�</p><p>��ȷ�Ͻ���ϵͳ�ǿ��õġ�</p>");
				return;
			}

			char *m_lpvBuffer = req.GetResult();
			pStr1 = strstr(m_lpvBuffer, "&reportParamsId=");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error("<p>��ȡ����ID����(1)</p>");
				return;
			}
			pStr2 = strstr(pStr1 + 16, "\r\n");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error("<p>��ȡ����ID����(2)</p>");
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
				Error("<p>ͨ��ID���ܲ�ѯʧ�ܡ�</p><p>��������֪���Ĵ���</p>");
				return;
			}
			m_lpvBuffer = req2.GetResult();
			pStr1 = strstr(m_lpvBuffer, "Exception: ");
			if (pStr1 != NULL)
			{
				pStr2 = strstr(pStr1, "at");
				if (pStr2 != NULL)
				{
					if (m_need_update_cookie)
						cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
					Error("<p>����ϵͳ�����ˣ����ܲ�ѯʧ�ܣ����Ժ�����~</p>");
					return;
				}
			}
			pStr1 = NULL;
			pStr2 = NULL;

			pStr1 = strstr(m_lpvBuffer, "com.runqian.report.view.text.TextFileServlet");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error("<p>���ܲ�ѯ���ز�������(1)</p>");
				return;
			}
			pStr2 = strstr(pStr1 + 46, "\";");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error("<p>���ܲ�ѯ���ز�������(2)</p>");
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
				Error("<p>���ܷ�����Ϣʧ�ܣ�</p>");
				return;
			}

			m_lpvBuffer = req3.GetResult();
			pStr1 = strstr(m_lpvBuffer, "����\t");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error("<p>ѧ��������ȡʧ�ܣ�(1)</p>");
				return;
			}
			pStr2 = strstr(pStr1 + 4, "\t\t");
			if (pStr2 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				Error("<p>ѧ��������ȡʧ�ܣ�(2)</p>");
				return;
			}

			if ((pStr2 - pStr1) <= 4)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error, 
					"<p><b>������ȡʧ���ˡ���ȷ��������Ϣ����ȷ�ġ�</b></p><p>���������ѧ��: %s</p>", 
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
					"<p><b>��ȡ��Ϣʧ�ܣ���ȷ��������ȷ��</b></p><p>���������ѧ��: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				return;
			}
			char m_xxmz_html[128] = { 0 };
			sprintf(m_xxmz_html, "<div class=\"weui-cells__title\">%s</div>", m_xxmz);

			pStr1 = NULL;
			pStr2 = NULL;
			pStr1 = strstr(m_lpvBuffer, "����ʱ��\t\t\n");
			if (pStr1 == NULL)
			{
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					"<p><b>���ܵ��ı���������⡣</b></p><p>���������ѧ��: %s</p>",
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
					"<p><b>�յ��ı����С�������⡣</b></p><p>���������ѧ��: %s</p>",
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
				pStr1 = pStr2 + 1; // ѧ��
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
				if (strstr(m_lb, "����") != NULL)
				{
					char m_kcmz_cx[256] = { 0 };
					strcat(m_kcmz_cx, "<b style=\"color:#f57c00\">[����]</b> ");
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
				Error("<p>��Ǹ�����ܲ�ѯ������ʧ�ܣ����Ժ����ԡ�</p>");
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
				if (m_interval >= 3) // ������γɼ��������3���£�����ʾ���ڳɼ�
				{
					if (m_test_info[i].date == m_max_date)
					{
						char m_temp[1024] = { 0 };
						sprintf(m_temp, QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
						m_list.append(m_temp);
					}
				}
				else if (m_test_info[i].date == m_max_date || m_test_info[i].date == m_secondary_max)
				{
					char m_temp[1024] = { 0 };
					sprintf(m_temp, QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
					m_list.append(m_temp);
				}
			}
			m_list.append("</div>");
		}

		cout << GLOBAL_HEADER;

		if (m_xhgs > 1)
		{
			std::string title("���˲�ѯ - ���ܳɼ���ѯ - ");
			title += SOFTWARE_NAME;

			cout << strformat( header.c_str(), title.c_str());
			cout << strformat( m_lpszQuery.c_str(), "���˲�ѯ", m_list.c_str());
		}
		else
		{
			std::string title(m_xxmz);
			title += " - ���ܳɼ���ѯ - ";
			title += SOFTWARE_NAME;

			cout << strformat( header.c_str(), title.c_str());
			cout << strformat( m_lpszQuery.c_str(), m_xxmz, m_list.c_str());
		}
		cout << footer.c_str();

		g_QueryCount = g_QueryCount + m_xhgs;
		g_fQueryCount = fopen("QueryCount.bin", "w+");
		if (g_fQueryCount != NULL)
		{
			fprintf(g_fQueryCount, "%ld", g_QueryCount);
			fclose(g_fQueryCount);
		}
}

// QQ�ʺŰ���������߼� (/OAuth2Assoc.fcgi)
void OAuth2_Association(bool isPOST)
{
	if (CGI_QUERY_STRING == NULL)
	{
		Error("�������� (Null QUERY_STRING)");
		return;
	}

	// ������߼�
	char *pStr1 = strstr(CGI_QUERY_STRING, "release=");
	if (pStr1 != NULL)
	{
		char student_id[512] = { 0 };
		get_student_id(student_id);
		if (student_id == NULL)
		{
			Error("�Ƿ������� (��δ��¼)");
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
			Error("�Ƿ������� (��ݴ���)");
			return;
		}

		// SQLite3 ���ݿ⣬���� main���� URLScoreHelper���ֶ� text id(36), text password(36), text openid(128)��
		/*char *query = new char[strlen("DELETE FROM URPScoreHelper WHERE id='") + 36 + 1];
		memset(query, 0, strlen("DELETE FROM URPScoreHelper WHERE id='") + 36 + 1);
		strcpy(query, "DELETE FROM URPScoreHelper WHERE id='");
		strcat(query, student_id);
		strcat(query, "';");*/
		std::string query("UPDATE URPScoreHelper SET openid=NULL WHERE id='");
		query += student_id;
		query += "';";

		char **db_Result = NULL;
		sqlite3_stmt *stmt;
		int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

		if (db_ret != SQLITE_OK)
		{
			std::string Err_Msg("<b>�����ʧ�ܣ����Ժ����ԡ�</b><p>(");
			Err_Msg += sqlite3_errmsg(db);
			Err_Msg += ")</p>";
			Error(Err_Msg.c_str());
			sqlite3_finalize(stmt);
			return;
		}

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			break;
		}

		sqlite3_finalize(stmt);
		cout << "Status: 302 Found\r\nLocation: main.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	pStr1 = strstr(CGI_QUERY_STRING, "openid=");
	if (pStr1 == NULL)
	{
		Error("��Ȩʧ�� (Null openid)");
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
		bool m_need_update_cookie = false;
		std::string m_photo(" "); // ��Ҫ��Ƭ
		process_cookie(&m_need_update_cookie, m_photo);

		if (!m_photo.empty()) // �������¼�ˣ��Ǿ��ߵ���ҳ��ȥ��
		{
			if (strcmp(CGI_SCRIPT_NAME, "/main.fcgi") == 0)
			{
				parse_main(m_need_update_cookie, m_photo);
				delete[]openid;
				return;
			}
			cout << "Status: 302 Found\r\nLocation: main.fcgi\r\n" << GLOBAL_HEADER;
			delete[]openid;
			return;
		}

		// ������� sid�����Զ���дѧ�š����Ҵ����ݿ��������롣
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
			// SQLite3 ���ݿ⣬���� main���� URLScoreHelper���ֶ� text id(36), text password(36), text openid(128)��
			std::string query("SELECT password FROM URPScoreHelper WHERE id='");
			query += stid;
			query += "';";

			char **db_Result = NULL;
			sqlite3_stmt *stmt;
			int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

			if (db_ret != SQLITE_OK)
			{
				char Err_Msg[512] = "<b>���ݿ�׼��ʧ�ܣ���ȷ�����ݿ�Ϸ��ԡ�</b><p>(";
				strcat(Err_Msg, sqlite3_errmsg(db));
				strcat(Err_Msg, ")</p>");
				Error(Err_Msg);
				sqlite3_finalize(stmt);
				delete[]openid;
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
		}

		std::string m_lpszHomepage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

		// �����ҳ
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";

		cout << GLOBAL_HEADER;

		std::string title("QQ�û��� - ");
		title += SOFTWARE_NAME;
		cout << strformat( header.c_str(), title.c_str());

		if (strlen(stid) == 0 || strcmp(stid, "NONE") == 0)
		{
			cout << strformat( m_lpszHomepage.c_str(), SOFTWARE_NAME, openid, "��лʹ��QQ��¼�����Ȱ��Լ���ѧ�Ű� :)",
				 "flex", "", "flex", pass);
		}
		else if(strlen(pass) == 0)
		{
			cout << strformat( m_lpszHomepage.c_str(), SOFTWARE_NAME, openid, "��лʹ��QQ��¼��������ѧ�Ŷ�Ӧ�������������� :)",
				"none", stid, "flex", pass);
		}
		else
		{
			cout << strformat( m_lpszHomepage.c_str(), SOFTWARE_NAME, openid, "��лʹ��QQ��¼����������֤������������ :)",
				"none", stid, "none", pass);
		}
		cout << footer.c_str();
	}
	else // �ύ�ʺ�������֤�룬�����¼����
	{
		// ��ȡ POST ���ݡ�
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0)
		{
			Error("<p>��������POST ���ݳ����쳣��</p>");
			delete[]openid;
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

		// ��ȡѧ��
		char *pStr1 = strstr(m_post_data, "xh=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error("<p>�޷���ȡѧ����Ϣ��</p>");
			delete[]openid;
			return;
		}
		char *pStr2 = strstr(pStr1 + 3, "&");
		char m_xuehao[128] = { 0 };
		mid(m_xuehao, pStr1, pStr2 - pStr1 - 3, 3);
		pStr1 = NULL;
		pStr2 = NULL;

		// ��ȡ����
		pStr1 = strstr(m_post_data, "mm=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error("<p>�޷���ȡ������Ϣ��</p>");
			delete[]openid;
			return;
		}
		pStr2 = strstr(pStr1 + 3, "&");
		char m_password[128] = { 0 };
		mid(m_password, pStr1, pStr2 - pStr1 - 3, 3);
		pStr1 = NULL;
		pStr2 = NULL;

		// ��ȡ��֤��
		pStr1 = strstr(m_post_data, "yzm=");
		if (pStr1 == NULL)
		{
			free(m_post_data);
			Error("<p>�޷���ȡ��֤����Ϣ��</p>");
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

		// �����ʾ��¼�ɹ���Ӧ��д�����ݿ��ˡ�
		// SQLite3 ���ݿ⣬���� main���� URLScoreHelper���ֶ� text id(36), text password(36), text openid(128)��
		std::string query("UPDATE URPScoreHelper SET openid='");
		query += openid;
		query += "' WHERE id='";
		query += m_xuehao;
		query += "';";

		char **db_Result = NULL;
		sqlite3_stmt *stmt;
		int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

		if (db_ret != SQLITE_OK)
		{
			char Err_Msg[1024] = "<b>�ܱ�Ǹ��QQ��ʧ�ܡ�</b><p>���ݿ���� (";
			strcat(Err_Msg, sqlite3_errmsg(db));
			strcat(Err_Msg, ")</p><p>���Ǳ�߹��������������¼�ġ�</p>");
			Error(Err_Msg);
			sqlite3_finalize(stmt);
			free(m_post_data);
			delete[]openid;
			return;
		}

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		cout << "Status: 302 Found\r\n";
		cout << "Location: main.fcgi\r\n";
		cout << GLOBAL_HEADER;
		
		free(m_post_data);
	}

	delete[]openid;
	return;
}

// ��ѧ����ҳ�� (/TeachEval.fcgi)
void parse_teaching_evaluation()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // �����ݣ���Ҫ��ȡ��Ƭ
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // ��û��½�Ͷ�ȥ��½��
	{
		cout << "Status: 302 Found\r\nLocation: index.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	// ����Ƿ���Ҫ��ѧ����
	CCurlTask req;
	if (!req.Exec(false, GET_TEACH_EVAL_LIST, CGI_HTTP_COOKIE))
	{
		Error("<p><b>��ѧ��������Ͷ��ʧ��</b></p><p>curl ����ʧ��</p>");
		return;
	}

	char *m_rep_body = req.GetResult();
	char *m_result = strstr(m_rep_body, "ѧ�������ʾ��б�");
	if (m_result == NULL)
	{
		Error("<p>�ӷ�������ȡ��ѧ������Ϣʧ�ܡ�</p>");
		return;
	}

	m_result = strstr(m_rep_body, "�ǽ�ѧ����ʱ�ڣ�������ʱ���ѹ�");
	if (m_result != NULL)
	{
		Error("<p>ѧԺ��û�п��������أ�������������Ŷ</p>");
		return;
	}

	int counts = 0;
	teach_eval te[200];
	char *m_result1 = strstr(m_rep_body, "<td align=\"center\">��</td>");
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
			Error("<p>�ӷ�������ȡ�����б�ʧ�ܡ�</p>");
			return;
		}
		char img_txt[128] = { 0 };
		mid(img_txt, m_result + 11, m_result2 - m_result - 11, 0);
		char dst[10][128] = {0};

		int split_ret = split(dst, img_txt, "#@");

		if (split_ret != 6)
		{
			Error("<p>������Ŀ��Ŀ�ָ�ʧ�ܡ�</p>");
			return;
		}

		strcpy(te[counts].wjbm, dst[0]);
		strcpy(te[counts].bpr, dst[1]);
		strcpy(te[counts].pgnr, dst[5]);
		strcpy(te[counts].name, dst[4]);

		counts++;
		m_result1 = strstr(m_result, "<td align=\"center\">��</td>");
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
				to_eval_list += "δ����";
				to_eval++;
			}
			else
			{
				to_eval_list += "<b style=\"color:#00a70e\">������</b>";
			}
			to_eval_list += "</div></div>";
	}
	to_eval_list += "</div>";

	std::string m_lpszTeachEvalPage = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	std::string outer;
	char out_head[1024] = { 0 };

	cout << GLOBAL_HEADER;

	std::string title = "һ������ - ";
	title += SOFTWARE_NAME;
	cout << strformat(header.c_str(), title.c_str());
	bool need_eval = true;
	if (to_eval && counts)
	{
		sprintf(out_head, 
			"<div class=\"weui-cells__title\">�ţ���ǰ���� %d �ſγ���Ҫ�������ܹ� %d �š�</div>", 
			to_eval, counts);
	}
	else
	{
		strcpy(out_head, "<div class=\"weui-cells__title\"><p>�ţ��㶼���ۺ��������Ǻñ��� O(��_��)O</div>");
		need_eval = false;
	}

	outer.append(out_head);
	outer.append(to_eval_list);

	cout << strformat(
		m_lpszTeachEvalPage.c_str(),
		need_eval ? "��ʦ�����࣬�������¡�Ĭ��ȫ�������㶮�� :)" : "",
		need_eval ? "block" : "none"
		, outer.c_str());
	cout << footer.c_str();
}

// ��ѧ�������� (POST /TeachEval.fcgi?act=Evaluate)
void teaching_evaluation()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // �����ݣ���Ҫ��ȡ��Ƭ
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // ��û��½�Ͷ�ȥ��½��
	{
		cout << "Status: 302 Found\r\nLocation: index.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	// ��ȡ POST ���ݡ�
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		Error("<p>��������POST ���ݳ����쳣��</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

	// ��ȡ��������
	char *pStr1 = strstr(m_post_data, "nr=");
	if (pStr1 == NULL)
	{
		free(m_post_data);
		Error("<p>�޷���ȡ�����������ݡ�</p>");
		return;
	}

	char zgpj[1024] = { 0 };
	left(zgpj, pStr1 + 3, m_post_length - 3);
	free(m_post_data);

	// ����Ƿ���Ҫ��ѧ����
	CCurlTask req;
	if (!req.Exec(false, GET_TEACH_EVAL_LIST, CGI_HTTP_COOKIE))
	{
		Error("<p><b>��ѧ��������Ͷ��ʧ��</b></p><p>curl ����ʧ��</p>");
		return;
	}

	char *m_rep_body = req.GetResult();
	char *m_result = strstr(m_rep_body, "ѧ�������ʾ��б�");
	if (m_result == NULL)
	{
		Error("<p>�ӷ�������ȡ��ѧ������Ϣʧ�ܡ�</p>");
		return;
	}

	m_result = strstr(m_rep_body, "�ǽ�ѧ����ʱ�ڣ�������ʱ���ѹ�");
	if (m_result != NULL)
	{
		Error("<p><b>��Ŷ����������</b></p><p>�ǽ�ѧ����ʱ�ڣ�������ʱ���ѹ���</p>");
		return;
	}

	int counts = 0;
	teach_eval te[200];
	char *m_result1 = strstr(m_rep_body, "<td align=\"center\">��</td>");
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
			Error("<p>�ӷ�������ȡ�����б�ʧ�ܡ�</p>");
			return;
		}
		char img_txt[128] = { 0 };
		mid(img_txt, m_result + 11, m_result2 - m_result - 11, 0);
		char dst[10][128] = { 0 };

		int split_ret = split(dst, img_txt, "#@");

		if (split_ret != 6)
		{
			Error("<p>������Ŀ��Ŀ�ָ�ʧ�ܡ�</p>");
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
		m_result1 = strstr(m_result, "<td align=\"center\">��</td>");
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
					Error("<p><b>׼������ʱ�����˴���</b></p><p>curl ����ʧ��</p>");
					return;
				}

				m_rep_body = req2.GetResult();
				char *m_result = strstr(m_rep_body, "�ʾ�����ҳ��");
				if (m_result == NULL)
				{
					std::string err_msg = "<p>������������</p><p>�ܱ�Ǹ����������";
					err_msg = err_msg + te[i].name + "���γ�ʱ�����˴���</p><p>(������ϸҳ��ʧ��)</p>";
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
						std::string err_msg = "<p>������������</p><p>�ܱ�Ǹ����������";
						err_msg = err_msg + te[i].name + "���γ�ʱ�����˴���</p><p>(������Ŀ���űպ�ʧ��)</p>";
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
						std::string err_msg = "<p>������������</p><p>�ܱ�Ǹ����������";
						err_msg = err_msg + te[i].name + "���γ�ʱ�����˴���</p><p>(ֵ��Ŀ���ſ���ʧ��)</p>";
						Error(err_msg.c_str());
						return;
					}
					char *p3 = strstr(p2 + 7, "\"");
					if (p2 == NULL)
					{
						std::string err_msg = "<p>������������</p><p>�ܱ�Ǹ����������";
						err_msg = err_msg + te[i].name + "���γ�ʱ�����˴���</p><p>(ֵ��Ŀ���űպ�ʧ��)</p>";
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

				// ������ſ��Ƿ���Ҫ��ѧ����
				CCurlTask req3;
				if (!req3.Exec(false, POST_TEACH_EVAL, CGI_HTTP_COOKIE, true, post_data.c_str()))
				{
					std::string err_msg = "<p>������������</p><p>�ܱ�Ǹ����������";
					err_msg = err_msg + te[i].name + "���γ�ʱ�����˴���</p><p>curl ����ʧ��</p>";
					Error(err_msg.c_str());
				}

				m_rep_body = req3.GetResult();
				m_result = strstr(m_rep_body, "�ɹ�");
				if (m_result == NULL)
				{
					std::string err_msg = "<p>������������</p><p>�ܱ�Ǹ����������";
					err_msg = err_msg + te[i].name + "���γ�ʱ�����˴���</p>";
					Error(err_msg.c_str());
					return;
				}
			}
		}
	}
	cout << "Status: 302 Found\r\nLocation: TeachEval.fcgi\r\n" << GLOBAL_HEADER;
	return;
}

// �޸�����ҳ�� (/changePassword.fcgi)
void parse_change_password()
{
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // �����ݣ���Ҫ��ȡ��Ƭ
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // ��û��½�Ͷ�ȥ��½��
	{
		cout << "Status: 302 Found\r\nLocation: index.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	std::string m_lpszQuery = ReadTextFileToMem(CGI_SCRIPT_FILENAME);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\r\n";
	cout << GLOBAL_HEADER;

	std::string title("�޸����� - ");
	title += SOFTWARE_NAME;

	cout << strformat( header.c_str(), title.c_str());
	cout << strformat( m_lpszQuery.c_str());

	cout << footer.c_str();
}

// �޸����� (POST /changePassword.fcgi)
void do_change_password() //(POST /changePassword.fcgi)
{
	// modifyPassWordAction.do?pwd=
	bool m_need_update_cookie = false;
	std::string m_photo(" "); // �����ݣ���Ҫ��ȡ��Ƭ
	process_cookie(&m_need_update_cookie, m_photo);

	if (m_photo.empty()) // ��û��½�Ͷ�ȥ��½��
	{
		cout << "Status: 302 Found\r\nLocation: index.fcgi\r\n" << GLOBAL_HEADER;
		return;
	}

	// ��ȡ POST ���ݡ�
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0 || m_post_length > 127)
	{
		Error("<p>��������POST ���ݳ����쳣��</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);

	// ��ȡ������
	char *pStr1 = strstr(m_post_data, "mm=");
	if (pStr1 == NULL)
	{
		free(m_post_data);
		Error("<p>�����˴����޷���ȡ POST ���ݡ�</p>");
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
		Error("<p>�����볤�Ȳ��ܳ���12���ַ���</p>");
		return;
	}

	char GET_RET[1024] = { 0 };
	sprintf(GET_RET, REQ_CHANGE_PASSWORD, pwd);

	CCurlTask req;
	if (!req.Exec(false, GET_RET, CGI_HTTP_COOKIE))
	{
		Error("<p><b>�޸�����ʱ�����˴���</b></p><p>curl ����ʧ��</p>");
		return;
	}

	// ��ȡ���ܽ��
	char *m_rep_header = req.GetResult();
	pStr1 = strstr(m_rep_header, "�ɹ�");
	if (pStr1 == NULL)
	{
		Error("<p>�����޸�ʧ�ܣ���ȷ���Ƿ������˷Ƿ��ַ��������Ժ����ԡ�</p>");
		return;
	}

	// SQLite3 ���ݿ⣬���� main���� URLScoreHelper���ֶ� text id(36), text password(36), text openid(128) text lastlogin(64)��
	std::string query("UPDATE URPScoreHelper SET password='");
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

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		std::string Err_Msg("<b>�����޸ĳɹ�������¼���ݿ��¼ʧ�ܣ����Ժ����ԡ�(��ʹ���������¼)</b><p>(");
		Err_Msg += sqlite3_errmsg(db);
		Err_Msg += ")</p>";
		Error(Err_Msg.c_str());
		sqlite3_finalize(stmt);
		return;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		break;
	}

	sqlite3_finalize(stmt);

	student_logout();
	cout << "Status: 302 Found\r\nLocation: index.fcgi\r\n" << GLOBAL_HEADER;
}