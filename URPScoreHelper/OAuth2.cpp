#include "stdafx.h"
#include "OAuth2.h"
#include "StringHelper.h"
#include "General.h"
#include "Encrypt.h"

using namespace std;

void getRedirectUri(char *http_host, char *m_Domain)
{
	if (CGI_HTTPS != NULL && strcmp(CGI_HTTPS, "") != 0
		&& strcmp(CGI_HTTPS, "off") != 0
		&& strcmp(CGI_HTTPS, "OFF") != 0
		&& strcmp(CGI_HTTPS, "0") != 0)
	{
		strcpy(m_Domain, "https://");
	}
	else
	{
		strcpy(m_Domain, "http://");
	}
	strcat(m_Domain, http_host);
	strcat(m_Domain, "/OAuth2CallBack.fcgi");
}

// 处理QQ登录入口请求
void OAuth2_process()
{
	char m_Domain[2048] = { 0 };
	if (CGI_HTTP_HOST == NULL)
	{
		cout << "Status: 500 Internal Server Error\r\n";
		Error("错误：缺少 HTTP_HOST 环境变量，请检查 FastCGI 接口设定。");
		return;
	}
	char *stid = NULL;
	if (CGI_QUERY_STRING != NULL)
	{
		char *pStr1 = strstr(CGI_QUERY_STRING, "stid=");
		if (pStr1 != NULL)
		{
			char *pStr2 = strstr(pStr1 + 5, "&");
			char *sid = new char[strlen(CGI_QUERY_STRING)];
			if (pStr2 == NULL)
			{
				right(sid, pStr1 + 5, strlen(CGI_QUERY_STRING) - 5);
			}
			else
			{
				mid(sid, pStr1 + 5, pStr2 - pStr1 - 5, 0);
			}
			DeCodeStr(sid);
			stid = sid;
		}
	}
	getRedirectUri(CGI_HTTP_HOST, m_Domain);
	int m_UrlEncodedLength;
	char *m_UrlEncodedDomain = url_encode(m_Domain, strlen(m_Domain), &m_UrlEncodedLength);
	strcpy(m_Domain, m_UrlEncodedDomain);
	free(m_UrlEncodedDomain);

	char *m_lpszURL = NULL;
	if (stid == NULL)
	{
		m_lpszURL = new char[strlen(OAUTH2_AUTHENTICATION) + strlen(OAUTH2_APPID) + strlen(m_Domain) + 5];
		sprintf(m_lpszURL, OAUTH2_AUTHENTICATION, OAUTH2_APPID, m_Domain, "NONE");
	}
	else if(strlen(stid) == 0)
	{
		m_lpszURL = new char[strlen(OAUTH2_AUTHENTICATION) + strlen(OAUTH2_APPID) + strlen(m_Domain) + 5];
		sprintf(m_lpszURL, OAUTH2_AUTHENTICATION, OAUTH2_APPID, m_Domain, "NONE");
		delete[]stid;
	}
	else
	{
		m_lpszURL = new char[strlen(OAUTH2_AUTHENTICATION) + strlen(OAUTH2_APPID) + strlen(m_Domain) + strlen(stid) + 1];
		sprintf(m_lpszURL, OAUTH2_AUTHENTICATION, OAUTH2_APPID, m_Domain, stid);
		delete[]stid;
	}

	cout << "Status: 302 Found\r\n";
	cout << "Location: " << m_lpszURL << "\r\n";
	cout << GLOBAL_HEADER;
	delete[]m_lpszURL;
}

// QQ授权回调
void OAuth2_CallBack()
{
	char m_Domain[4096] = { 0 };
	if (CGI_HTTP_HOST == NULL)
	{
		Error("错误：缺少 HTTP_HOST 环境变量，请检查 FastCGI 接口设定。");
		return;
	}
	getRedirectUri(CGI_HTTP_HOST, m_Domain);

	if (CGI_QUERY_STRING == NULL)
	{
		Error("鉴权失败 (Null QUERY_STRING)");
		return;
	}
	char *pStr1 = strstr(CGI_QUERY_STRING, "code=");
	if (pStr1 == NULL)
	{
		Error("鉴权失败 (Null Code)");
		return;
	}
	char *pStr2 = strstr(pStr1 + 5, "&");
	char *code = new char[strlen(CGI_QUERY_STRING)];
	if (pStr2 == NULL)
	{
		right(code, pStr1 + 5, strlen(CGI_QUERY_STRING) - 5);
	}
	else
	{
		mid(code, pStr1 + 5, pStr2 - pStr1 - 5, 0);
	}

	char *access_token_req = new char[strlen(OAUTH2_ACCESS_TOKEN) +
		strlen(OAUTH2_APPID) +
		strlen(OAUTH2_SECRET) +
		+ strlen(m_Domain)
		+ strlen(code) + 1];

	sprintf(access_token_req, OAUTH2_ACCESS_TOKEN, OAUTH2_APPID, OAUTH2_SECRET, code, m_Domain);

	CURL* curl = curl_easy_init();

	if (curl == NULL)
	{
		Error("无法初始化 curl");
		delete[]access_token_req;
		delete[]code;
		return;
	}

	std::string html;
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OAuth2_curl_receive);
	curl_easy_setopt(curl, CURLOPT_URL, access_token_req);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
	CURLcode ret = curl_easy_perform(curl);

	if (ret != CURLE_OK)
	{
		Error("curl 操作失败");
		delete[]access_token_req;
		delete[]code;
		curl_easy_cleanup(curl);
		return;
	}

	pStr1 = strstr((char *)html.c_str(), "access_token=");
	if (pStr1 == NULL)
	{
		std::string err("<p><b>无法读取 access_token</b></p><p>");
		err.append(html);
		err.append("</p>");
		Error(err.c_str());
		delete[]access_token_req;
		delete[]code;
		curl_easy_cleanup(curl);
		return;
	}

	pStr2 = strstr(pStr1 + 14, "&");
	if (pStr2 == NULL)
	{
		Error("获取 access_token 失败！(Json_right)");
		delete[]access_token_req;
		delete[]code;
		curl_easy_cleanup(curl);
		return;
	}

	char *access_token = new char[html.length()];
	memset(access_token, 0, html.length());
	mid(access_token, pStr1 + 13, pStr2 - pStr1 - 13, 0);

	/*pStr1 = strstr(html, "\"social_uid\": ");
	if (pStr1 == NULL)
	{
		cout << "Status: 500 Internal Server Error\r\n";
		Error(html);
		free(html);
		delete[]access_token_req;
		delete[]code;
		return;
	}

	pStr2 = strstr(pStr1 + 15, ",");
	if (pStr2 == NULL)
	{
		cout << "Status: 500 Internal Server Error\r\n";
		Error("获取 access_token 失败！(Json_right)");
		free(html);
		delete[]access_token_req;
		delete[]code;
		return;
	}

	char *openid = new char[strlen(html)];
	memset(openid, 0, strlen(html));
	mid(openid, pStr1 + 14, pStr2 - pStr1 - 14, 0);*/
	html.erase();
	curl_easy_cleanup(curl);

	curl = curl_easy_init();
	if (curl == NULL)
	{
		Error("无法初始化 curl");
		delete[]access_token_req;
		delete[]code;
		return;
	}
	char openid_req[512] = { 0 };
	sprintf(openid_req, OAUTH2_GET_OPENID, access_token);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OAuth2_curl_receive);
	curl_easy_setopt(curl, CURLOPT_URL,openid_req);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
	ret = curl_easy_perform(curl);

	if (ret != CURLE_OK)
	{
		Error("curl 操作失败");
		delete[]access_token_req;
		delete[]code;
		curl_easy_cleanup(curl);
		return;
	}
	curl_easy_cleanup(curl);

	pStr1 = strstr((char *)html.c_str(), "\"openid\":\"");
	if (pStr1 == NULL)
	{
		std::string err("<p><b>无法读取 openid</b></p><p>");
		err.append(html);
		err.append("</p>");
		Error(err.c_str());
		delete[]access_token_req;
		delete[]code;
		return;
	}

	pStr2 = strstr(pStr1 + 11, "\"");
	if (pStr2 == NULL)
	{
		Error("获取 access_token 失败！(Json_right)");
		delete[]access_token_req;
		delete[]code;
		return;
	}

	char *openid = new char[html.length()];
	memset(openid, 0, html.length());
	mid(openid, pStr1 + 10, pStr2 - pStr1 - 10, 0);

	//  成功拿到 access_token 和 openid

	// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
	std::string query("SELECT id, password FROM URPScoreHelper WHERE openid='");
	query += openid;
	query += "';";

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	int db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		char Err_Msg[512] = "<b>数据库准备失败！请确认数据库合法性。</b><p>(";
		strcat(Err_Msg, sqlite3_errmsg(db));
		strcat(Err_Msg, ")</p>");
		Error(Err_Msg);
		sqlite3_finalize(stmt);
		delete[]access_token;
		delete[]access_token_req;
		delete[]openid;
		delete[]code;
		return;
	}

	const unsigned char *id = NULL;
	const unsigned char *password = NULL;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		id = sqlite3_column_text(stmt, 0);
		password = sqlite3_column_text(stmt, 1);
		break;
	}

	if (id == NULL || password == NULL) // 无记录，跳转至 OAuth2Assoc.fcgi
	{
		pStr1 = strstr(CGI_QUERY_STRING, "state=");
		char id[128] = { 0 };
		if (pStr1 != NULL)
		{
			pStr2 = strstr(pStr1 + 6, "&");
			if (pStr2 == NULL)
			{
				right(id, pStr1 + 6, strlen(CGI_QUERY_STRING) - 6);
			}
			else
			{
				mid(id, pStr1 + 6, pStr2 - pStr1 - 6, 0);
			}
			EnCodeStr(id, id);
		}

		cout << "Status: 302 Found\r\n";
		if (strcmp(id, "NONE") == 0)
		{
			cout << "Location: OAuth2Assoc.fcgi?openid=" << openid << "\r\n";
		}
		else
		{
			cout << "Location: OAuth2Assoc.fcgi?openid=" << openid << "&stid=" << id << "\r\n";
		}
		cout << GLOBAL_HEADER;
		sqlite3_finalize(stmt);
		delete[]access_token;
		delete[]access_token_req;
		delete[]openid;
		delete[]code;
		return;
	}

	char *encrypt_pass = new char[strlen((char *)password) * 4 + 1];
	char *encrypt_id = new char[strlen((char *)id) * 4 + 1];
	strcpy(encrypt_id, (char *)id);
	strcpy(encrypt_pass, (char *)password);
	EnCodeStr(encrypt_id, encrypt_id);
	EnCodeStr(encrypt_pass, encrypt_pass);
	cout << "Status: 302 Found\r\n"
		<< "Location: index.fcgi?id=" << encrypt_id << "&pass=" << encrypt_pass << "\r\n"
		<< GLOBAL_HEADER;
	
	sqlite3_finalize(stmt);
	delete[]encrypt_id;
	delete[]encrypt_pass;
	delete[]access_token;
	delete[]access_token_req;
	delete[]openid;
	delete[]code;
}

size_t OAuth2_curl_receive(char *buffer, size_t size, size_t nmemb, std::string *html)
{
	int block_size = size * nmemb;
	html->append(buffer);
	return block_size;
}