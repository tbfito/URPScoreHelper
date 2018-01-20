#include "headers.h"
#include "OAuth2.h"
#include "StringHelper.h"
#include "General.h"
#include "Encrypt.h"

void getRedirectUri(char *buffer, size_t bufflen)
{
	memset(buffer, 0, bufflen);
	strncpy(buffer, getAppURL().c_str(), bufflen - 1);
	strcat(buffer, "/OAuth2CallBack.fcgi");
}

// 处理微信登录入口请求
void OAuth2_process()
{
	char m_Domain[4096];
	char *stid = NULL;

	std::string str_stid = _GET(std::string(CGI_QUERY_STRING), "stid");
	if (!str_stid.empty())
	{
		size_t qslen = strlen(CGI_QUERY_STRING);
		char *sid = new char[qslen];
		memset(sid, 0, qslen);
		strncpy(sid, str_stid.c_str(), qslen - 1);
		DeCodeStr(sid);
		stid = sid;
	}

	getRedirectUri(m_Domain, sizeof(m_Domain) - 1);
	int m_UrlEncodedLength = 0;
	char *m_UrlEncodedDomain = url_encode(m_Domain, strlen(m_Domain), &m_UrlEncodedLength);
	memset(m_Domain, 0, sizeof(m_Domain));
	strncpy(m_Domain, m_UrlEncodedDomain, sizeof(m_Domain) - 1);
	free(m_UrlEncodedDomain);

	char *m_lpszURL = NULL;
	if (stid == NULL) // 如果没有传入学号
	{
		m_lpszURL = new char[strlen(OAUTH2_AUTHENTICATION) + strlen(OAUTH2_APPID) + strlen(m_Domain) + 5];
		sprintf(m_lpszURL, OAUTH2_AUTHENTICATION, OAUTH2_APPID, m_Domain, "NONE");
	}
	else if(strlen(stid) == 0)  // 如果学号无效
	{
		m_lpszURL = new char[strlen(OAUTH2_AUTHENTICATION) + strlen(OAUTH2_APPID) + strlen(m_Domain) + 5];
		sprintf(m_lpszURL, OAUTH2_AUTHENTICATION, OAUTH2_APPID, m_Domain, "NONE");
		delete[]stid;
	}
	else // 传入了学号
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

// 微信授权回调
void OAuth2_CallBack()
{
	char m_Domain[4096] = { 0 };
	getRedirectUri(m_Domain, sizeof(m_Domain) - 1);

	std::string str_code = _GET(std::string(CGI_QUERY_STRING), "code");
	if (str_code.empty())
	{
		Error(u8"鉴权失败 (Null Code)");
		return;
	}
	size_t qslen = strlen(CGI_QUERY_STRING);
	char *code = new char[qslen];
	memset(code, 0, qslen);
	strncpy(code, str_code.c_str(), qslen - 1);

	char *access_token_req = new char[strlen(OAUTH2_ACCESS_TOKEN) +
		strlen(OAUTH2_APPID) +
		strlen(OAUTH2_SECRET) +
		/* [FOR QQ] + strlen(m_Domain) */
		+ strlen(code) + 1];

	sprintf(access_token_req, OAUTH2_ACCESS_TOKEN, OAUTH2_APPID, OAUTH2_SECRET, code /* [FOR QQ] , m_Domain */);

	CURL* curl = curl_easy_init();

	if (curl == NULL)
	{
		Error(u8"网络通信异常");
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
		Error(u8"网络通信异常");
		delete[]access_token_req;
		delete[]code;
		curl_easy_cleanup(curl);
		return;
	}

	char *pStr1 = strstr((char *)html.c_str(), "\"access_token\":\"");
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

	char *pStr2 = strstr(pStr1 + 17, "\"");
	if (pStr2 == NULL)
	{
		Error(u8"<p><b>无法读取 access_token</b></p><p>");
		delete[]access_token_req;
		delete[]code;
		curl_easy_cleanup(curl);
		return;
	}

	curl_easy_cleanup(curl);

	char *access_token = new char[html.length()];
	memset(access_token, 0, html.length());
	mid(access_token, pStr1 + 16, pStr2 - pStr1 - 16, 0);

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
		Error(u8"<p><b>无法读取 openid</b></p><p>");
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
		std::string str_state = _GET(std::string(CGI_QUERY_STRING), "state");
		char id_encrypt[128] = { 0 };
		char id_state[128] = { 0 };
		char orign_id_state[128] = { 0 };
		if (!str_state.empty())
		{
			strncpy(id_state, str_state.c_str(), sizeof(id_state) - 1);
			strncpy(orign_id_state, id_state, sizeof(orign_id_state) - 1);
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

	char encrypt_pass[512] = { 0 };
	char encrypt_id[512] = { 0 };
	strncpy(encrypt_id, (char *)id, sizeof(encrypt_id) - 1);
	strncpy(encrypt_pass, (char *)password, sizeof(encrypt_pass) - 1);
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
	sprintf(get_user_info_req, OAUTH2_GET_USER_INFO, access_token, openid);
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
	strncpy(html_str, html.c_str(), html.length());
	replace_string(html_str, "\\/", "/");

	char *pStr1 = strstr(html_str, "\"nickname\":\"");
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
	mid(nickname, pStr1 + 12, pStr2 - pStr1 - 12, 0);
	
	pStr1 = strstr(html_str, "\"headimgurl\":\"");
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
	mid(avatar, pStr1 + 14, pStr2 - pStr1 - 14, 0);
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
bool GetOAuthUserInfo(char *student_id, char *nickname, char *avatar_url, size_t nickname_bufflen, size_t avatar_url_bufflen)
{
	memset(nickname, 0, nickname_bufflen);
	memset(avatar_url, 0, avatar_url_bufflen);

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

	strncpy(nickname, OAuth_name, nickname_bufflen - 1);
	strncpy(avatar_url, OAuth_avatar, avatar_url_bufflen - 1);

	return true;
}

size_t OAuth2_curl_receive(char *buffer, size_t size, size_t nmemb, std::string *html)
{
	size_t block_size = size * nmemb;
	html->append(buffer);
	return block_size;
}