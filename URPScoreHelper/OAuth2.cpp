#include "stdafx.h"
#include "OAuth2.h"
#include "StringHelper.h"
#include "General.h"
#include "Encrypt.h"

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
		Error(u8"错误：缺少 HTTP_HOST 环境变量，请检查 FastCGI 接口设定。");
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
		Error(u8"错误：缺少 HTTP_HOST 环境变量，请检查 FastCGI 接口设定。");
		return;
	}
	getRedirectUri(CGI_HTTP_HOST, m_Domain);

	if (CGI_QUERY_STRING == NULL)
	{
		Error(u8"鉴权失败 (Null QUERY_STRING)");
		return;
	}
	char *pStr1 = strstr(CGI_QUERY_STRING, "code=");
	if (pStr1 == NULL)
	{
		Error(u8"鉴权失败 (Null Code)");
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
		Error(u8"无法初始化 curl");
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
		Error(u8"curl 操作失败");
		delete[]access_token_req;
		delete[]code;
		curl_easy_cleanup(curl);
		return;
	}

	pStr1 = strstr((char *)html.c_str(), "access_token=");
	if (pStr1 == NULL)
	{
		std::string err(u8"<p><b>无法读取 access_token</b></p><p>");
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
		Error(u8"获取 access_token 失败！(Json_right)");
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
		Error(u8"获取 access_token 失败！(Json_right)");
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
		Error(u8"无法初始化 curl");
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
		Error(u8"curl 操作失败");
		delete[]access_token_req;
		delete[]code;
		curl_easy_cleanup(curl);
		return;
	}
	curl_easy_cleanup(curl);

	pStr1 = strstr((char *)html.c_str(), "\"openid\":\"");
	if (pStr1 == NULL)
	{
		std::string err(u8"<p><b>无法读取 openid</b></p><p>");
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
		Error(u8"获取 access_token 失败！(Json_right)");
		delete[]access_token_req;
		delete[]code;
		return;
	}

	char *openid = new char[html.length()];
	memset(openid, 0, html.length());
	mid(openid, pStr1 + 10, pStr2 - pStr1 - 10, 0);

	//  成功拿到 access_token 和 openid
	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[1];
	MYSQL_BIND query_ret[2];
	memset(bind, 0, sizeof(bind));
	memset(query_ret, 0, sizeof(query_ret));
	std::string query("SELECT `id`, `password` FROM `UserInfo` WHERE `openid`=?");

	if (stmt == NULL)
	{
		char Err_Msg[1024] = u8"<b>数据库操作失败</b><p>(statement 初始化失败)</p>";
		Error(Err_Msg);
		delete[]access_token;
		delete[]access_token_req;
		delete[]openid;
		delete[]code;
		return;
	}

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)openid;
	bind[0].buffer_length = strlen(openid);

	char id[36] = { 0 };
	query_ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	query_ret[0].buffer = (void *)id;
	query_ret[0].buffer_length = sizeof(id);

	char password[36] = { 0 };
	query_ret[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	query_ret[1].buffer = (void *)password;
	query_ret[1].buffer_length = sizeof(password);

	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
		mysql_stmt_bind_param(stmt, bind) != 0 || 
		mysql_stmt_bind_result(stmt, query_ret) != 0 || 
		mysql_stmt_execute(stmt) != 0 || 
		mysql_stmt_store_result(stmt) != 0
		)
	{
		mysql_stmt_close(stmt);
		char Err_Msg[1024] = u8"<b>数据库操作失败</b><p>(";
		strcat(Err_Msg, mysql_error(&db));
		strcat(Err_Msg, ")</p>");
		Error(Err_Msg);
		delete[]access_token;
		delete[]access_token_req;
		delete[]openid;
		delete[]code;
		return;
	}

	while (mysql_stmt_fetch(stmt) == 0);

	mysql_stmt_close(stmt);

	if (strlen(id) == 0 || strlen(password) == 0) // 无记录，跳转至 OAuth2Assoc.fcgi
	{
		pStr1 = strstr(CGI_QUERY_STRING, "state=");
		char id_encrypt[128] = { 0 };
		char id_state[128] = { 0 };
		char orign_id_state[128] = { 0 };
		if (pStr1 != NULL)
		{
			pStr2 = strstr(pStr1 + 6, "&");
			if (pStr2 == NULL)
			{
				right(id_state, pStr1 + 6, strlen(CGI_QUERY_STRING) - 6);
			}
			else
			{
				mid(id_state, pStr1 + 6, pStr2 - pStr1 - 6, 0);
			}
			strcpy(orign_id_state, id_state);
			EnCodeStr(id_state, id_state);
		}

		char encrypt_access_token[1024] = { 0 };
		EnCodeStr(access_token, encrypt_access_token);

		cout << "Status: 302 Found\r\n";
		if (strcmp(orign_id_state, "NONE") == 0) //如果 OAuth State 传过来的学号不存在 (为 NONE)
		{
			if (strlen(id) != 0) // 但通过 openid 却能查到数据库中对应的学号
			{
				EnCodeStr(id, id_encrypt);
				cout << "Location: " << getAppURL().c_str() << "/OAuth2Assoc.fcgi?openid=" << openid << "&stid=" << id_encrypt << "&proc=" << encrypt_access_token << "\r\n";
			}
			else // 完全不存在学号，则为新绑定用户
			{
				cout << "Location: " << getAppURL().c_str() << "/OAuth2Assoc.fcgi?openid=" << openid << "&proc=" << encrypt_access_token << "\r\n";
			}
		}
		else // OAuth State 传来有效的需要绑定的学号
		{
			cout << "Location: " << getAppURL().c_str() << "/OAuth2Assoc.fcgi?openid=" << openid << "&stid=" << id_state << "&proc=" << encrypt_access_token << "\r\n";
		}
		cout << GLOBAL_HEADER;

		delete[]access_token;
		delete[]access_token_req;
		delete[]openid;
		delete[]code;
		return;
	}

	char encrypt_pass[512];
	char encrypt_id[512];
	strcpy(encrypt_id, (char *)id);
	strcpy(encrypt_pass, (char *)password);
	EnCodeStr(encrypt_id, encrypt_id);
	EnCodeStr(encrypt_pass, encrypt_pass);
	WriteOAuthUserInfo(access_token, openid, (char *)id);
	cout << "Status: 302 Found\r\n"
		<< "Location: " << getAppURL().c_str() << "/index.fcgi?stid=" << encrypt_id << "&pass=" << encrypt_pass << "\r\n"
		<< GLOBAL_HEADER;
	delete[]access_token;
	delete[]access_token_req;
	delete[]openid;
	delete[]code;
}

// 向数据库中写入第三方账户的昵称与头像URL
void WriteOAuthUserInfo(char *access_token, char *openid, char *student_id)
{
	CURL *curl = curl_easy_init();
	if (curl == NULL)
	{
		return;
	}
	char get_user_info_req[512] = { 0 };
	std::string html;
	sprintf(get_user_info_req, OAUTH2_GET_USER_INFO, access_token, OAUTH2_APPID, openid);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OAuth2_curl_receive);
	curl_easy_setopt(curl, CURLOPT_URL, get_user_info_req);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
	CURLcode ret = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	if (ret != CURLE_OK)
	{
		return;
	}
	char *html_str = (char *)malloc(html.length() + 1);
	memset(html_str, 0, html.length() + 1);
	strcpy(html_str, html.c_str());
	replace_string(html_str, "\\/", "/");
	
	char *pStr1 = strstr(html_str, "\"nickname\": \"");
	if (pStr1 == NULL)
	{
		free(html_str);
		return;
	}
	char *pStr2 = strstr(pStr1 + 14, "\"");
	if (pStr2 == NULL)
	{
		free(html_str);
		return;
	}
	char *nickname = new char[html.length()];
	memset(nickname, 0, html.length());
	mid(nickname, pStr1 + 13, pStr2 - pStr1 - 13, 0);
	
	pStr1 = strstr(html_str, "\"figureurl_qq_1\": \"");
	if (pStr1 == NULL)
	{
		delete[]nickname;
		free(html_str);
		return;
	}
	pStr2 = strstr(pStr1 + 20, "\"");
	if (pStr2 == NULL)
	{
		delete[]nickname;
		free(html_str);
		return;
	}
	char *avatar = new char[html.length()];
	memset(avatar, 0, html.length());
	mid(avatar, pStr1 + 19, pStr2 - pStr1 - 19, 0);
	free(html_str);

	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[3];
	memset(bind, 0, sizeof(bind));
	std::string query("UPDATE `UserInfo` SET `OAuth_name`=?, `OAuth_avatar`=? WHERE `id`=?");

	if (stmt == NULL)
	{
		delete[]nickname;
		delete[]avatar;
		return;
	}

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)nickname;
	bind[0].buffer_length = strlen(nickname);

	bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[1].buffer = (void *)avatar;
	bind[1].buffer_length = strlen(avatar);

	bind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[2].buffer = (void *)student_id;
	bind[2].buffer_length = strlen(student_id);

	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
		mysql_stmt_bind_param(stmt, bind) != 0 || 
		mysql_stmt_execute(stmt) != 0)
	{
		mysql_stmt_close(stmt);
		delete[]nickname;
		delete[]avatar;
		return;
	}

	mysql_stmt_close(stmt);
	delete[]nickname;
	delete[]avatar;
}

// 读取第三方账户昵称与头像URL
bool GetOAuthUserInfo(char *student_id, char *nickname, char *avatar_url)
{
	MYSQL_STMT *stmt = mysql_stmt_init(&db);
	MYSQL_BIND bind[1];
	MYSQL_BIND query_ret[2];
	memset(bind, 0, sizeof(bind));
	memset(query_ret, 0, sizeof(query_ret));
	std::string query("SELECT `OAuth_name`, `OAuth_avatar` FROM `UserInfo` WHERE `id`=?");

	if (stmt == NULL)
	{
		return false;
	}

	bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	bind[0].buffer = (void *)student_id;
	bind[0].buffer_length = strlen(student_id);

	char OAuth_name[1024] = { 0 };
	query_ret[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	query_ret[0].buffer = (void *)OAuth_name;
	query_ret[0].buffer_length = sizeof(OAuth_name);

	char OAuth_avatar[4096] = { 0 };
	query_ret[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	query_ret[1].buffer = (void *)OAuth_avatar;
	query_ret[1].buffer_length = sizeof(OAuth_avatar);

	if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0 || 
		mysql_stmt_bind_param(stmt, bind) != 0 || 
		mysql_stmt_bind_result(stmt, query_ret) != 0 || 
		mysql_stmt_execute(stmt) != 0 || 
		mysql_stmt_store_result(stmt) != 0
		)
	{
		mysql_stmt_close(stmt);
		return false;
	}

	while (mysql_stmt_fetch(stmt) == 0);

	mysql_stmt_close(stmt);

	if (strlen(OAuth_name) == 0 || strlen(OAuth_avatar) == 0)
	{
		return false;
	}

	strcpy(nickname, OAuth_name);
	strcpy(avatar_url, OAuth_avatar);

	return true;
}

size_t OAuth2_curl_receive(char *buffer, size_t size, size_t nmemb, std::string *html)
{
	int block_size = size * nmemb;
	html->append(buffer);
	return block_size;
}