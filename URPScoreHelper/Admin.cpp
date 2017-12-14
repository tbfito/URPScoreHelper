﻿#include "headers.h"
#include "Admin.h"
#include "General.h"
#include "StringHelper.h"
#include "Encrypt.h"
#include "gbkutf8.h"

// admin 控制器入口
void admin_intro()
{
	if (strcmp(CGI_SCRIPT_NAME, "/admin/login.fcgi") == 0)
	{
		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
		{
			parse_admin_login();
			return;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
		{
			do_admin_login();
			return;
		}
	}

	if (strcmp(CGI_SCRIPT_NAME, "/admin/settings.fcgi") == 0)
	{
		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
		{
			parse_admin_settings();
			return;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
		{
			save_admin_settings();
			return;
		}
	}

	if (strcmp(CGI_SCRIPT_NAME, "/admin/info.fcgi") == 0)
	{
		parse_admin_info();
		return;
	}

	if (strcmp(CGI_SCRIPT_NAME, "/admin/change-pass.fcgi") == 0)
	{
		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
		{
			parse_admin_change_password();
			return;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
		{
			do_admin_change_password();
			return;
		}
	}

	if (strcmp(CGI_SCRIPT_NAME, "/admin/homepage-notice.fcgi") == 0)
	{
		homepage_notice();
		return;
	}

	if (strcmp(CGI_SCRIPT_NAME, "/admin/set-discussion.fcgi") == 0)
	{
		set_discussion();
		return;
	}

	if (strcmp(CGI_SCRIPT_NAME, "/admin/find-user.fcgi") == 0)
	{
		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
		{
			parse_find_user();
			return;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
		{
			do_find_user();
			return;
		}
	}

	if (strcmp(CGI_SCRIPT_NAME, "/admin/adv-card.fcgi") == 0)
	{
		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
		{
			parse_admin_adv_card();
			return;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
		{
			change_admin_adv_card();
			return;
		}
	}

	if (strcmp(CGI_SCRIPT_NAME, "/admin/maintenance.fcgi") == 0)
	{
		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
		{
			site_maintenance();
			return;
		}
		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
		{
			site_maintenance();
			return;
		}
	}

	if (strcmp(CGI_REQUEST_URI, "/admin/index.fcgi") == 0 || strcmp(CGI_REQUEST_URI, "/admin") == 0)
	{
		cout << "Status: 301 Moved Permanently\r\n" << "Location: " << getAppURL().c_str() << "/admin/\r\n" << GLOBAL_HEADER;
		return;
	}

	if (strcmp(CGI_REQUEST_URI, "/admin/") == 0)
	{
		parse_admin_index();
		return;
	}

	cout << "Status: 404 Not Found\r\n";
	admin_error(u8"页面未找到或发生未知错误");
}

// 处理管理员登录入口 (GET /admin/login.fcgi)
void parse_admin_login()
{
	if (strstr(CGI_QUERY_STRING, "act=logout") != NULL)
	{
		cout << "Status: 302 Found\r\n"
			 << "Set-Cookie: admin_sessid=; max-age=-1; path=/admin/\r\n"
			 << "Location: " << getAppURL().c_str() << "/admin/login.fcgi\r\n"
			 << GLOBAL_HEADER;
		return;
	}

	if (verify_session())
	{
		cout << "Status: 302 Found\r\n"
			 << "Location: " << getAppURL().c_str() << "/admin/index.fcgi\r\n"
			 << GLOBAL_HEADER;
		return;
	}

	cout << GLOBAL_HEADER
	     << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(),
			 APP_NAME, APP_NAME, SOFTWARE_NAME, SOFTWARE_COPYRIGHT).c_str();
}

// 后台管理错误提示
void admin_error(const char *err_msg)
{
	cout << GLOBAL_HEADER;
	char *pStr = strstr(CGI_SCRIPT_FILENAME, "\\");
	bool isUnixBasedPath = (pStr == NULL);
	if (isUnixBasedPath)
	{
		pStr = strstr(CGI_SCRIPT_FILENAME, "/");
	}
	if (pStr == NULL)
	{
		cout << err_msg;
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
	strcat(file_root, "tips.fcgi");
	delete[]doc_root;
	cout << strformat(ReadTextFileToMem(file_root).c_str(), APP_NAME, APP_NAME, err_msg).c_str();
	delete[]file_root;
}

// 处理管理员录 (POST /admin/login.fcgi)
void do_admin_login()
{
	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0 || m_post_length > 64)
	{
		admin_error(u8"<p><b>POST前方高能</b></p><p>非官方人员请迅速撤离</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	std::string m_user = _POST(post, "usr");
	std::string m_password = _POST(post, "pwd");
	if (m_user.empty() || m_password.empty())
	{
		admin_error(u8"<p>帐号或密码信息获取失败</p>");
		return;
	}

	int len = url_decode((char *)m_user.c_str(), m_user.length());
	char temp1[128] = { 0 };
	left(temp1, (char *)m_user.c_str(), len);
	m_user = temp1;
	len = url_decode((char *)m_password.c_str(), m_password.length());
	char temp2[128] = { 0 };
	left(temp2, (char *)m_password.c_str(), len);
	m_password = temp2;


	// 判断管理员登录
	if (strcmp(m_user.c_str(), ADMIN_USER_NAME) != 0 || strcmp(m_password.c_str(), ADMIN_PASSWORD) != 0)
	{
		admin_error(u8"<p><b>あなたは一体誰ですか</b></p><p>非官方人员请迅速撤离</p>");
		return;
	}

	cout << "Status: 302 Found\r\n"
		 << "Set-Cookie: admin_sessid=" << generate_session().c_str() << "; max-age=600; path=/admin/\r\n"
		 << "Location: " << getAppURL().c_str() << "/admin/index.fcgi\r\n"
		 << GLOBAL_HEADER;
}

// 生成登录会话 Session，由 sessino() 调用
std::string generate_session()
{
	unsigned long long result = std::time(nullptr);
	result += 600;
	char tmp[128] = { 0 };
	sprintf(tmp, "%lld", result);
	std::string ret;
	ret = ret + ADMIN_USER_NAME + "-" + ADMIN_PASSWORD + "-" + tmp;
	char encoded[1024] = { 0 };
	EnCodeStr((char *)ret.c_str(), encoded);
	std::string txt(encoded);
	return txt;
}

// 验证登录会话 Session，由 sessino() 调用
bool verify_session()
{
	char *session = (char *)malloc(1024);
	memset(session, 0, 1024);

	char *pStr1 = strstr(CGI_HTTP_COOKIE, "admin_sessid=");
	if (pStr1 != NULL)
	{
		char *pStr2 = strstr(pStr1 + 13, ";");
		if (pStr2 == NULL) // 如果这条 Cookie 在最后一条
		{
			right(session, CGI_HTTP_COOKIE, strlen(CGI_HTTP_COOKIE) - (pStr1 - CGI_HTTP_COOKIE) - 13);
		}
		else
		{
			mid(session, pStr1, pStr2 - pStr1 - 13, 13);
		}
	}
	else
	{
		free(session);
		return false;
	}

	DeCodeStr(session);

	std::string tmp;
	tmp = tmp + ADMIN_USER_NAME + "-" + ADMIN_PASSWORD + "-";
	char *pStrX = strstr(session, tmp.c_str());
	if (pStrX == NULL)
	{
		free(session);
		return false;
	}
	char tmp1[128] = { 0 };
	right(tmp1, session, strlen(session) - tmp.length());

	char *unused;
	unsigned long long timestamp = strtoull(tmp1, &unused, 10);
	unsigned long long now = std::time(nullptr);
	if (timestamp <= now) // 会话超时
	{
		free(session);
		return false;
	}

	free(session);
	return true;
}

// Session 处理，返回 true 代表可以继续向下操作，返回 false 代表需要立即返回。
bool session()
{
	if (!verify_session())
	{
		cout << "Status: 302 Found\r\n"
			<< "Location: " << getAppURL().c_str() << "/admin/login.fcgi\r\n"
			<< GLOBAL_HEADER;
		return false;
	}
	else // 如果已登录，那么这是一个新操作，更新cookie过期时间
	{
		cout << "Set-Cookie: admin_sessid=" << generate_session().c_str() << "; max-age=600; path=/admin/\r\n";
		return true;
	}
}

// 处理管理模板首页 (GET /admin/ /admin/index.fcgi)
void parse_admin_index()
{
	if (!session())
		return;

	cout << GLOBAL_HEADER
		 << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, APP_NAME).c_str();
}

// 处理站点信息页面 (GET /admin/settings.fcgi)
void parse_admin_settings()
{
	if (!session())
		return;

	cout << GLOBAL_HEADER
		<< strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(),
			APP_NAME, APP_NAME, SECONDARY_TITLE, APP_DESCRIPTION, APP_KEYWORDS, SERVER_URL,
			USER_AGENT, CURL_TIMEOUT, CURL_USE_PROXY ? 1 : 0, CURL_PROXY_URL, ENABLE_QUICK_QUERY ? 1 : 0,
			OAUTH2_APPID, OAUTH2_SECRET, FOOTER_TEXT, ANALYSIS_CODE).c_str();
}

// 保存站点信息 (POST /admin/settings.fcgi)
void save_admin_settings()
{
	if (!session())
		return;

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		admin_error(u8"<p><b>POST错误</b></p><p>提交的数据可能存在问题</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	std::string m_APP_NAME = _POST(post, "APP_NAME");
	std::string m_SECONDARY_TITLE = _POST(post, "SECONDARY_TITLE");
	std::string m_APP_DESCRIPTION = _POST(post, "APP_DESCRIPTION");
	std::string m_APP_KEYWORDS = _POST(post, "APP_KEYWORDS");
	std::string m_SERVER_URL = _POST(post, "SERVER_URL");
	std::string m_USER_AGENT = _POST(post, "USER_AGENT");
	std::string m_CURL_TIMEOUT = _POST(post, "CURL_TIMEOUT");
	std::string m_CURL_USE_PROXY = _POST(post, "CURL_USE_PROXY");
	std::string m_CURL_PROXY_URL = _POST(post, "CURL_PROXY_URL");
	std::string m_ENABLE_QUICK_QUERY = _POST(post, "ENABLE_QUICK_QUERY");
	std::string m_OAUTH2_APPID = _POST(post, "OAUTH2_APPID");
	std::string m_OAUTH2_SECRET = _POST(post, "OAUTH2_SECRET");
	std::string m_FOOTER_TEXT = _POST(post, "FOOTER_TEXT");
	std::string m_ANALYSIS_CODE = _POST(post, "ANALYSIS_CODE");

	decode_post_data(m_APP_NAME);
	decode_post_data(m_SECONDARY_TITLE);
	decode_post_data(m_APP_DESCRIPTION);
	decode_post_data(m_APP_KEYWORDS);
	decode_post_data(m_SERVER_URL);
	decode_post_data(m_USER_AGENT);
	decode_post_data(m_CURL_TIMEOUT);
	decode_post_data(m_CURL_USE_PROXY);
	decode_post_data(m_CURL_PROXY_URL);
	decode_post_data(m_ENABLE_QUICK_QUERY);
	decode_post_data(m_OAUTH2_APPID);
	decode_post_data(m_OAUTH2_SECRET);
	decode_post_data(m_FOOTER_TEXT);
	decode_post_data(m_ANALYSIS_CODE);

	UpdateSettings("APP_NAME", m_APP_NAME.c_str());
	UpdateSettings("SECONDARY_TITLE", m_SECONDARY_TITLE.c_str());
	UpdateSettings("APP_DESCRIPTION", m_APP_DESCRIPTION.c_str());
	UpdateSettings("APP_KEYWORDS", m_APP_KEYWORDS.c_str());
	UpdateSettings("SERVER_URL", m_SERVER_URL.c_str());
	UpdateSettings("USER_AGENT", m_USER_AGENT.c_str());
	UpdateSettings("CURL_TIMEOUT", m_CURL_TIMEOUT.c_str());
	UpdateSettings("CURL_USE_PROXY", m_CURL_USE_PROXY.c_str());
	UpdateSettings("CURL_PROXY_URL", m_CURL_PROXY_URL.c_str());
	UpdateSettings("ENABLE_QUICK_QUERY", m_ENABLE_QUICK_QUERY.c_str());
	UpdateSettings("OAUTH2_APPID", m_OAUTH2_APPID.c_str());
	UpdateSettings("OAUTH2_SECRET", m_OAUTH2_SECRET.c_str());
	UpdateSettings("FOOTER_TEXT", m_FOOTER_TEXT.c_str());
	UpdateSettings("ANALYSIS_CODE", m_ANALYSIS_CODE.c_str());

	// 因为header(部分)内容一直缓存在内存中，所以需要单独对其进行更新。(footer 内容每请求都会在 LoadConfig() 当中更新)
	header = strformat(ReadTextFileToMem(HEADER_TEMPLATE_LOCATION).c_str(), "%s", m_SECONDARY_TITLE.c_str(), m_APP_KEYWORDS.c_str(), m_APP_DESCRIPTION.c_str());
	footer = strformat(ReadTextFileToMem(FOOTER_TEMPLATE_LOCATION).c_str(), m_APP_NAME.c_str(), m_FOOTER_TEXT.c_str(), SOFTWARE_NAME, m_ANALYSIS_CODE.c_str());

	admin_error(u8"设定已保存");
}

// 获取 POST 中的内容(缺陷：表单名称不能中不能有重叠)
std::string _POST(std::string & post, const char *name)
{
	std::string ret;
	size_t len = strlen(name);
	size_t pos1 = post.find(name);
	size_t pos2 = post.find("&", pos1 + len + 1);
	if (pos1 == std::string::npos)
	{
		return ret;
	}
	if (pos2 == std::string::npos)
	{
		ret = post.substr(pos1 + len + 1, post.length() - pos1 - len - 1);
	}
	else
	{
		ret = post.substr(pos1 + len + 1, pos2 - pos1 - len - 1);
	}
	return ret;
}

// 更新设置表
void UpdateSettings(const char *name, const char *value)
{
	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	if (stmt == NULL)
	{
		return;
	}

	MYSQL_BIND bind[2];
	memset(bind, 0, sizeof(bind));
	std::string query("UPDATE `Settings` SET `value`=? WHERE `name`=?");

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)value;
	bind[0].buffer_length = strlen(value);

	bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[1].buffer = (void *)name;
	bind[1].buffer_length = strlen(name);

	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
		mysql_stmt_bind_param(stmt, bind) != 0 || 
		mysql_stmt_execute(stmt) != 0
		)
	{
		mysql_stmt_close(stmt);
		return;
	}
	mysql_stmt_close(stmt);
}

// 解码URL编码
void decode_post_data(std::string & str)
{
	int len = url_decode((char *)str.c_str(), str.length());
	char temp[4096] = { 0 };
	left(temp, (char *)str.c_str(), len);
	str = temp;
}

// 处理修改管理员信息页面 (GET /admin/change-pass.fcgi)
void parse_admin_change_password()
{
	if (!session())
		return;

	cout << GLOBAL_HEADER
		<< strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, ADMIN_USER_NAME).c_str();
}

// 修改管理员信息
void do_admin_change_password()
{
	if (!session())
		return;

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		admin_error(u8"<p><b>POST错误</b></p><p>提交的数据可能存在问题</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	std::string m_ADMIN_USER_NAME = _POST(post, "ADMIN_USER_NAME");
	std::string m_ORIG_PASSWORD = _POST(post, "ORIG_PASSWORD");
	std::string m_ADMIN_PASSWORD = _POST(post, "ADMIN_PASSWORD");
	std::string m_RENEW_PASSWORD = _POST(post, "RENEW_PASSWORD");

	decode_post_data(m_ADMIN_USER_NAME);
	decode_post_data(m_ORIG_PASSWORD);
	decode_post_data(m_ADMIN_PASSWORD);
	decode_post_data(m_RENEW_PASSWORD);

	if (m_ADMIN_USER_NAME.empty() || m_ORIG_PASSWORD.empty() || m_ADMIN_PASSWORD.empty() || m_RENEW_PASSWORD.empty())
	{
		admin_error(u8"输入有误，请重新输入");
		return;
	}
	if (m_ORIG_PASSWORD.length() < 5 || m_ADMIN_PASSWORD.length() < 5 || m_RENEW_PASSWORD.length() < 5)
	{
		admin_error(u8"新密码长度不能小于5位");
		return;
	}
	if (strcmp(m_ORIG_PASSWORD.c_str(), ADMIN_PASSWORD) != 0)
	{
		admin_error(u8"原始密码输入错误");
		return;
	}
	if (m_ORIG_PASSWORD == m_ADMIN_PASSWORD)
	{
		admin_error(u8"新旧密码不能一样");
		return;
	}
	if (m_ADMIN_PASSWORD != m_RENEW_PASSWORD)
	{
		admin_error(u8"确认密码与新密码不一致");
		return;
	}

	UpdateSettings("ADMIN_USER_NAME", m_ADMIN_USER_NAME.c_str());
	UpdateSettings("ADMIN_PASSWORD", m_ADMIN_PASSWORD.c_str());

	admin_error(u8"修改成功");

}

// 处理广告轮播页面 (GET /admin/adv-card.fcgi)
void parse_admin_adv_card()
{
	if (!session())
		return;

	cout << GLOBAL_HEADER
		 << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME,
			CARD_AD_BANNER_1_IMG, CARD_AD_BANNER_1_URL, CARD_AD_BANNER_2_IMG,
			CARD_AD_BANNER_2_URL).c_str();
}

// 修改广告轮播信息
void change_admin_adv_card()
{
	if (!session())
		return;

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		admin_error(u8"<p><b>POST错误</b></p><p>提交的数据可能存在问题</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	std::string m_CARD_AD_BANNER_1_IMG = _POST(post, "CARD_AD_BANNER_1_IMG");
	std::string m_CARD_AD_BANNER_1_URL = _POST(post, "CARD_AD_BANNER_1_URL");
	std::string m_CARD_AD_BANNER_2_IMG = _POST(post, "CARD_AD_BANNER_2_IMG");
	std::string m_CARD_AD_BANNER_2_URL = _POST(post, "CARD_AD_BANNER_2_URL");

	decode_post_data(m_CARD_AD_BANNER_1_IMG);
	decode_post_data(m_CARD_AD_BANNER_1_URL);
	decode_post_data(m_CARD_AD_BANNER_2_IMG);
	decode_post_data(m_CARD_AD_BANNER_2_URL);

	UpdateSettings("CARD_AD_BANNER_1_IMG", m_CARD_AD_BANNER_1_IMG.c_str());
	UpdateSettings("CARD_AD_BANNER_1_URL", m_CARD_AD_BANNER_1_URL.c_str());
	UpdateSettings("CARD_AD_BANNER_2_IMG", m_CARD_AD_BANNER_2_IMG.c_str());
	UpdateSettings("CARD_AD_BANNER_2_URL", m_CARD_AD_BANNER_2_URL.c_str());

	admin_error(u8"修改成功");

}

// 处理系统信息页面 (GET /admin/info.fcgi)
void parse_admin_info()
{
	if (!session())
		return;

	if (strcmp(CGI_QUERY_STRING, "act=reset_query_counter") == 0)
	{
		UpdateSettings("QueryCounter", "0");
		admin_error(u8"操作成功");
		return;
	}

	char *server_software = FCGX_GetParam("SERVER_SOFTWARE", request.envp);

	cout << GLOBAL_HEADER
		 << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME,
			 SOFTWARE_NAME, g_users, g_QueryCounter,
			 (server_software == NULL) ? "Unknown" : server_software, mysql_get_client_info(), mysql_get_server_info(&db),
			__DATE__, __TIME__, SOFTWARE_COPYRIGHT).c_str();
}

// 处理查找用户 (GET /admin/find-user.fcgi)
void parse_find_user()
{
	if (!session())
		return;

	char temp[1024] = { 0 };
	sprintf(temp, find_user_result_section, "", "", "", "", "", "");

	cout << GLOBAL_HEADER
		<< strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, "", temp, "", temp).c_str();
}

// 处理查找用户
void do_find_user()
{
	if (!session())
		return;

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		admin_error(u8"<p><b>POST错误</b></p><p>提交的数据可能存在问题</p>");
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);
	FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
	std::string post(m_post_data);
	free(m_post_data);

	if (strcmp(CGI_QUERY_STRING, "order=id") == 0)
	{
		std::string m_STUDENT_ID = _POST(post, "STUDENT_ID");

		decode_post_data(m_STUDENT_ID);

		MYSQL_STMT *stmt = mysql_stmt_init(&db);
		MYSQL_BIND bind[1];
		MYSQL_BIND query_ret[6];
		memset(bind, 0, sizeof(bind));
		memset(query_ret, 0, sizeof(query_ret));
		std::string query("SELECT `id`, `password`, `name`, `openid`, `OAuth_name`, `lastlogin` FROM `UserInfo` WHERE `id`=?");

		char tmp1[36] = { 0 };
		char tmp2[36] = { 0 };
		char tmp3[36] = { 0 };
		char tmp4[1024] = { 0 };
		char tmp5[1024] = { 0 };
		char tmp6[1024] = { 0 };

		bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[0].buffer = (void *)m_STUDENT_ID.c_str();
		bind[0].buffer_length = m_STUDENT_ID.length();

		query_ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[0].buffer = (void *)tmp1;
		query_ret[0].buffer_length = sizeof(tmp1);
		query_ret[1].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[1].buffer = (void *)tmp2;
		query_ret[1].buffer_length = sizeof(tmp2);
		query_ret[2].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[2].buffer = (void *)tmp3;
		query_ret[2].buffer_length = sizeof(tmp3);
		query_ret[3].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[3].buffer = (void *)tmp4;
		query_ret[3].buffer_length = sizeof(tmp4);
		query_ret[4].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[4].buffer = (void *)tmp5;
		query_ret[4].buffer_length = sizeof(tmp5);
		query_ret[5].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[5].buffer = (void *)tmp6;
		query_ret[5].buffer_length = sizeof(tmp6);

		if (stmt != NULL)
		{
			mysql_stmt_prepare(stmt, query.c_str(), query.length());
			mysql_stmt_bind_param(stmt, bind);
			mysql_stmt_bind_result(stmt, query_ret);
			mysql_stmt_execute(stmt);
			mysql_stmt_store_result(stmt);
			while (mysql_stmt_fetch(stmt) == 0);
			mysql_stmt_close(stmt);
		}

		if (strlen(tmp1) == 0)
		{
			strcpy(tmp1, u8"未找到");
		}

		char temp[1024] = { 0 };
		sprintf(temp, find_user_result_section, "", "", "", "", "", "");

		char* result = (char *)malloc(4096);
		memset(result, 0, 4096);
		sprintf(result, find_user_result_section, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6);

		cout << GLOBAL_HEADER
			<< strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME,
				m_STUDENT_ID.c_str(), result, "", temp).c_str();

		free(result);
		return;
	}
	else if (strcmp(CGI_QUERY_STRING, "order=name") == 0)
	{
		std::string m_STUDENT_NAME = _POST(post, "STUDENT_NAME");

		decode_post_data(m_STUDENT_NAME);

		MYSQL_STMT *stmt = mysql_stmt_init(&db);
		MYSQL_BIND bind[1];
		MYSQL_BIND query_ret[6];
		memset(bind, 0, sizeof(bind));
		memset(query_ret, 0, sizeof(query_ret));
		std::string query("SELECT `id`, `password`, `name`, `openid`, `OAuth_name`, `lastlogin` FROM `UserInfo` WHERE `name`=?");

		char tmp1[36] = { 0 };
		char tmp2[36] = { 0 };
		char tmp3[36] = { 0 };
		char tmp4[1024] = { 0 };
		char tmp5[1024] = { 0 };
		char tmp6[1024] = { 0 };

		bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		bind[0].buffer = (void *)m_STUDENT_NAME.c_str();
		bind[0].buffer_length = m_STUDENT_NAME.length();

		query_ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[0].buffer = (void *)tmp1;
		query_ret[0].buffer_length = sizeof(tmp1);
		query_ret[1].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[1].buffer = (void *)tmp2;
		query_ret[1].buffer_length = sizeof(tmp2);
		query_ret[2].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[2].buffer = (void *)tmp3;
		query_ret[2].buffer_length = sizeof(tmp3);
		query_ret[3].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[3].buffer = (void *)tmp4;
		query_ret[3].buffer_length = sizeof(tmp4);
		query_ret[4].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[4].buffer = (void *)tmp5;
		query_ret[4].buffer_length = sizeof(tmp5);
		query_ret[5].buffer_type = MYSQL_TYPE_VAR_STRING;
		query_ret[5].buffer = (void *)tmp6;
		query_ret[5].buffer_length = sizeof(tmp6);

		std::string result_str;

		if (stmt != NULL)
		{
			mysql_stmt_prepare(stmt, query.c_str(), query.length());
			mysql_stmt_bind_param(stmt, bind);
			mysql_stmt_bind_result(stmt, query_ret);
			mysql_stmt_execute(stmt);
			mysql_stmt_store_result(stmt);
			while (mysql_stmt_fetch(stmt) == 0)
			{
				char* result = (char *)malloc(4096);
				memset(result, 0, 4096);
				
				sprintf(result, find_user_result_section, *query_ret[0].is_null ? "" : tmp1,
														  *query_ret[1].is_null ? "" : tmp2,
														  *query_ret[2].is_null ? "" : tmp3,
														  *query_ret[3].is_null ? "" : tmp4,
														  *query_ret[4].is_null ? "" : tmp5,
														  *query_ret[5].is_null ? "" : tmp6
						);

				result_str.append(result);
				free(result);
			}
			mysql_stmt_close(stmt);
		}

		if (result_str.length() == 0)
		{
			strcpy(tmp1, u8"未找到");
			result_str = strformat(find_user_result_section, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6);
		}

		char temp[1024] = { 0 };
		sprintf(temp, find_user_result_section, "", "", "", "", "", "");

		cout << GLOBAL_HEADER
			<< strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, "", temp,
				m_STUDENT_NAME.c_str(), result_str.c_str()).c_str();

		return;
	}
	admin_error(u8"发生未知错误");
}

// 处理首页公告 (GET/POST /admin/homepage-notice.fcgi)
void homepage_notice()
{
	if (!session())
		return;

	if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
	{
		cout << GLOBAL_HEADER;
		cout << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, HOMEPAGE_NOTICE).c_str();
		return;
	}
	else if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
	{
		// 获取 POST 数据。
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0 || m_post_length > 10240)
		{
			admin_error(u8"<p><b>POST错误</b></p><p>提交的数据可能存在问题</p>");
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
		std::string post(m_post_data);
		free(m_post_data);

		std::string m_HOMEPAGE_NOTICE = _POST(post, "HOMEPAGE_NOTICE");
		decode_post_data(m_HOMEPAGE_NOTICE);
		UpdateSettings("HOMEPAGE_NOTICE", m_HOMEPAGE_NOTICE.c_str());

		admin_error(u8"修改成功");
		return;
	}
	admin_error(u8"发生未知错误");
}

// 处理交流讨论页面 (GET/POST /set-discussion.fcgi)
void set_discussion()
{
	if (!session())
		return;

	if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
	{
		cout << GLOBAL_HEADER;
		cout << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, 
				DISCUSSION_PAGE_CONTENT, DISCUSSION_PAGE_CODE).c_str();
		return;
	}
	else if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
	{
		// 获取 POST 数据。
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0)
		{
			admin_error(u8"<p><b>POST错误</b></p><p>提交的数据可能存在问题</p>");
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
		std::string post(m_post_data);
		free(m_post_data);

		std::string m_DISCUSSION_PAGE_CONTENT = _POST(post, "DISCUSSION_PAGE_CONTENT");
		std::string m_DISCUSSION_PAGE_CODE = _POST(post, "DISCUSSION_PAGE_CODE");

		decode_post_data(m_DISCUSSION_PAGE_CONTENT);
		decode_post_data(m_DISCUSSION_PAGE_CODE);

		UpdateSettings("DISCUSSION_PAGE_CONTENT", m_DISCUSSION_PAGE_CONTENT.c_str());
		UpdateSettings("DISCUSSION_PAGE_CODE", m_DISCUSSION_PAGE_CODE.c_str());

		admin_error(u8"修改成功");
		return;
	}
	admin_error(u8"发生未知错误");
}

// 处理维护模式页面 (GET/POST /maintenance.fcgi)
void site_maintenance()
{
	if (!session())
		return;

	if (strcmp(CGI_REQUEST_METHOD, "GET") == 0)
	{
		cout << GLOBAL_HEADER;
		cout << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, SITE_MAINTENANCE).c_str();
		return;
	}
	else if (strcmp(CGI_REQUEST_METHOD, "POST") == 0)
	{
		// 获取 POST 数据。
		int m_post_length = atoi(CGI_CONTENT_LENGTH);
		if (m_post_length <= 0)
		{
			admin_error(u8"<p><b>POST错误</b></p><p>提交的数据可能存在问题</p>");
			return;
		}
		char *m_post_data = (char *)malloc(m_post_length + 2);
		FCGX_GetLine(m_post_data, m_post_length + 1, request.in);
		std::string post(m_post_data);
		free(m_post_data);

		std::string m_SITE_MAINTENANCE = _POST(post, "SITE_MAINTENANCE");

		decode_post_data(m_SITE_MAINTENANCE);

		UpdateSettings("SITE_MAINTENANCE", m_SITE_MAINTENANCE.c_str());

		admin_error(u8"修改成功");
		return;
	}
	admin_error(u8"发生未知错误");
}