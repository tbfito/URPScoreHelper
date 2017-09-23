#include "stdafx.h"
#include "Admin.h"
#include "General.h"
#include "StringHelper.h"
#include "Encrypt.h"
#include "gbkutf8.h"

// �������Ա��¼��� (GET /admin/login.fcgi)
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

	cout << "Allow: GET, POST\r\n" << GLOBAL_HEADER
	     << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, APP_NAME).c_str();
}

// ��̨���������ʾ
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

// �������Ա¼ (POST /admin/login.fcgi)
void do_admin_login()
{
	// ��ȡ POST ���ݡ�
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0 || m_post_length > 64)
	{
		admin_error(u8"<p><b>POSTǰ������</b></p><p>�ǹٷ���Ա��Ѹ�ٳ���</p>");
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
		admin_error(u8"<p>�ʺŻ�������Ϣ��ȡʧ��</p>");
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


	// �жϹ���Ա��¼
	if (strcmp(m_user.c_str(), ADMIN_USER_NAME) != 0 || strcmp(m_password.c_str(), ADMIN_PASSWORD) != 0)
	{
		admin_error(u8"<p><b>���ʤ���һ���l�Ǥ���</b></p><p>�ǹٷ���Ա��Ѹ�ٳ���</p>");
		return;
	}

	cout << "Status: 302 Found\r\n"
		 << "Set-Cookie: admin_sessid=" << generate_session().c_str() << "; max-age=600; path=/admin/\r\n"
		 << "Location: " << getAppURL().c_str() << "/admin/index.fcgi\r\n"
		 << GLOBAL_HEADER;
}

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

bool verify_session()
{
	char *session = (char *)malloc(1024);
	memset(session, 0, 1024);

	char *pStr1 = strstr(CGI_HTTP_COOKIE, "admin_sessid=");
	if (pStr1 != NULL)
	{
		char *pStr2 = strstr(pStr1 + 13, ";");
		if (pStr2 == NULL) // ������� Cookie �����һ��
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
	if (timestamp <= now) // �Ự��ʱ
	{
		free(session);
		return false;
	}

	free(session);
	return true;
}

// �������ģ����ҳ (GET /admin/ /admin/index.fcgi)
void parse_admin_index()
{
	if (!verify_session())
	{
		cout << "Status: 302 Found\r\n"
			 << "Location: " << getAppURL().c_str() << "/admin/login.fcgi\r\n"
			 << GLOBAL_HEADER;
		return;
	}

	cout << GLOBAL_HEADER
		 << strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(), APP_NAME, APP_NAME).c_str();
}

// ����վ����Ϣҳ�� (GET /admin/settings.fcgi)
void parse_admin_settings()
{
	if (!verify_session())
	{
		cout << "Status: 302 Found\r\n"
			<< "Location: " << getAppURL().c_str() << "/admin/login.fcgi\r\n"
			<< GLOBAL_HEADER;
		return;
	}

	cout << GLOBAL_HEADER
		<< strformat(ReadTextFileToMem(CGI_SCRIPT_FILENAME).c_str(),
			APP_NAME, APP_NAME, SECONDARY_TITLE, APP_DESCRIPTION, APP_KEYWORDS, SERVER_URL,
			USER_AGENT, CURL_TIMEOUT, CURL_USE_PROXY ? 1 : 0, CURL_PROXY_URL, 
			OAUTH2_APPID, OAUTH2_SECRET, FOOTER_TEXT, ANALYSIS_CODE).c_str();
}

// ����վ����Ϣ (POST /admin/settings.fcgi)
void save_admin_settings()
{
	// ��ȡ POST ���ݡ�
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		admin_error(u8"<p><b>POST����</b></p><p>�ύ�����ݿ��ܴ�������</p>");
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
	UpdateSettings("OAUTH2_APPID", m_OAUTH2_APPID.c_str());
	UpdateSettings("OAUTH2_SECRET", m_OAUTH2_SECRET.c_str());
	UpdateSettings("FOOTER_TEXT", m_FOOTER_TEXT.c_str());
	UpdateSettings("ANALYSIS_CODE", m_ANALYSIS_CODE.c_str());

	// ��Ϊheader(����)��footer(ȫ��)����һֱ�������ڴ��У�������Ҫ����������и��¡�
	header = strformat(ReadTextFileToMem(HEADER_TEMPLATE_LOCATION).c_str(), "%s", m_SECONDARY_TITLE.c_str(), m_APP_KEYWORDS.c_str(), m_APP_DESCRIPTION.c_str());
	footer = strformat(ReadTextFileToMem(FOOTER_TEMPLATE_LOCATION).c_str(), m_APP_NAME.c_str(), m_FOOTER_TEXT.c_str(), SOFTWARE_NAME, m_ANALYSIS_CODE.c_str());

	admin_error(u8"�趨�ѱ���");
}

// ��ȡ POST �е�����
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

// �������ñ�
void UpdateSettings(const char *name, const char *value)
{
	std::string query("UPDATE Settings SET value='");
	query += value;
	query += "' WHERE name='";
	query += name;
	query += "';";

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		sqlite3_finalize(stmt);
		return;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		sqlite3_column_text(stmt, 0);
		break;
	}
	sqlite3_finalize(stmt);
}

// ����URL����
void decode_post_data(std::string & str)
{
	int len = url_decode((char *)str.c_str(), str.length());
	char temp[4096] = { 0 };
	left(temp, (char *)str.c_str(), len);
	str = temp;
}