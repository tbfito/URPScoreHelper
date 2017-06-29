#include "stdafx.h"
#include "OAuth2.h"
#include "StringHelper.h"
#include "General.h"
#include "CrawlFactory.h"

using namespace std;
int shift = 0;

void getRedirectUri(char *http_host, char *m_Domain)
{
	if (CGI_HTTPS != NULL && strcmp(CGI_HTTPS, "") != 0
		&& strcmp(CGI_HTTPS, "off") != 0
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
	char m_Domain[512] = { 0 };
	if (CGI_HTTP_HOST == NULL)
	{
		cout << "Status: 500 Internal Server Error\r\n";
		Error("错误：缺少 HTTP_HOST 环境变量，请检查 CGI 接口设定。");
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
	}
	else
	{
		m_lpszURL = new char[strlen(OAUTH2_AUTHENTICATION) + strlen(OAUTH2_APPID) + strlen(m_Domain) + strlen(stid) + 1];
		sprintf(m_lpszURL, OAUTH2_AUTHENTICATION, OAUTH2_APPID, m_Domain, stid);
	}

	cout << "Status: 302 Found\r\n";
	cout << "Location: " << m_lpszURL << "\r\n";
	cout << GLOBAL_HEADER;
	delete[]m_lpszURL;
}

// QQ授权回调
void OAuth2_CallBack()
{
	char m_Domain[512] = { 0 };
	if (CGI_HTTP_HOST == NULL)
	{
		Error("错误：缺少 HTTP_HOST 环境变量，请检查 CGI 接口设定。");
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
		Error("无法初始化 libcurl。");
		delete[]access_token_req;
		delete[]code;
		return;
	}

	char* html = (char *)malloc(1024);
	memset(html, 0, 1024);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, html);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_receive);
	curl_easy_setopt(curl, CURLOPT_URL, access_token_req);
	CURLcode ret = curl_easy_perform(curl);

	if (ret != CURLE_OK)
	{
		Error("curl 操作失败！");
		free(html);
		delete[]access_token_req;
		delete[]code;
		return;
	}

	pStr1 = strstr(html, "access_token=");
	if (pStr1 == NULL)
	{
		Error(html);
		free(html);
		delete[]access_token_req;
		delete[]code;
		return;
	}

	pStr2 = strstr(pStr1 + 14, "&");
	if (pStr2 == NULL)
	{
		Error("获取 access_token 失败！(Json_right)");
		free(html);
		delete[]access_token_req;
		delete[]code;
		return;
	}

	char *access_token = new char[strlen(html)];
	memset(access_token, 0, strlen(html));
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
	free(html);

	html = (char *)malloc(1024);
	memset(html, 0, 1024);
	char openid_req[512] = { 0 };
	sprintf(openid_req, OAUTH2_GET_OPENID, access_token);
	shift = 0;
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, html);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_receive);
	curl_easy_setopt(curl, CURLOPT_URL,openid_req);
	ret = curl_easy_perform(curl);

	if (ret != CURLE_OK)
	{
		cout << "Status: 500 Internal Server Error\r\n";
		Error("curl 操作失败！");
		free(html);
		delete[]access_token_req;
		delete[]code;
		return;
	}

	pStr1 = strstr(html, "\"openid\":\"");
	if (pStr1 == NULL)
	{
		cout << "Status: 500 Internal Server Error\r\n";
		Error(html);
		free(html);
		delete[]access_token_req;
		delete[]code;
		return;
	}

	pStr2 = strstr(pStr1 + 11, "\"");
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
	mid(openid, pStr1 + 10, pStr2 - pStr1 - 10, 0);

	//  成功拿到 access_token 和 openid

	sqlite3 * db = NULL;
	int db_ret = sqlite3_open("URPScoreHelper.db", &db);
	if (db_ret != SQLITE_OK)
	{
		cout << "Status: 500 Internal Server Error\r\n";
		Error("打开数据库文件失败，请检查 URPScoreHelper.db 是否存在。");
		free(html);
		delete[]access_token;
		delete[]access_token_req;
		delete[]openid;
		delete[]code;
		return;
	}

	// SQLite3 数据库，库名 main，表 URLScoreHelper，字段 text id(36), text password(36), text openid(128)。
	std::string query("SELECT id, password FROM URPScoreHelper WHERE openid='");
	query += openid;
	query += "';";

	char **db_Result = NULL;
	sqlite3_stmt *stmt;
	db_ret = sqlite3_prepare(db, query.c_str(), query.length(), &stmt, 0);

	if (db_ret != SQLITE_OK)
	{
		cout << "Status: 500 Internal Server Error\r\n";
		char Err_Msg[512] = "<b>数据库准备失败！请确认数据库合法性。</b><p>(";
		strcat(Err_Msg, sqlite3_errmsg(db));
		strcat(Err_Msg, ")</p>");
		Error(Err_Msg);
		free(html);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
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
		char id[36] = { 0 };
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
		free(html);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		delete[]access_token;
		delete[]access_token_req;
		delete[]openid;
		delete[]code;
		return;
	}

	cout << "Status: 302 Found\r\n"
		<< "Location: index.fcgi?id=" << id << "&pass=" << password << "\r\n"
		<< GLOBAL_HEADER;
	
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	delete[]access_token;
	delete[]access_token_req;
	delete[]openid;
	delete[]code;
	free(html);
	curl_easy_cleanup(curl);
}

size_t curl_receive(void *buffer, size_t size, size_t nmemb, char *html)
{
	int resize = size * nmemb;
	html = (char *)realloc(html, resize + shift + 1);
	memcpy(html + shift, (char *)buffer, resize);
	shift += resize;
	return resize;
}