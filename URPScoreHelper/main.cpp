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
#include "main.h"
#include "General.h"
#include "gbkutf8.h"
#include "INIReader.h"

int main(int argc, const char *argv[])
{
	initialize();
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
	fastcgi_app_intro();
	printf("%s\n%s\n\n%s\n", SOFTWARE_NAME, SOFTWARE_COPYRIGHT, "\tOptions: [-p (localhost):port_number]");

	cleanup();
	return 0;
}

void initialize()
{
	strcat(GLOBAL_HEADER, "Content-Type: text/html; charset=utf-8\r\nX-Powered-By: ");
	strcat(GLOBAL_HEADER, SOFTWARE_NAME);
	strcat(GLOBAL_HEADER, "\r\n\r\n");

	strcat(GLOBAL_HEADER_NO_CACHE_PLAIN_TEXT, "Content-Type: text/plain; charset=utf-8\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nExpires: Thu, 16 Oct 1997 00:00:00 GMT\r\nX-Powered-By: ");
	strcat(GLOBAL_HEADER_NO_CACHE_PLAIN_TEXT, SOFTWARE_NAME);
	strcat(GLOBAL_HEADER_NO_CACHE_PLAIN_TEXT, "\r\n\r\n");

	FCGX_Init();
	curl_global_init(CURL_GLOBAL_ALL);
	mysql_init(&db);

	INIReader reader("Database.ini");
	if (reader.ParseError() != 0) {
		strncpy(MYSQL_HOST, "127.0.0.1", sizeof(MYSQL_HOST) - 1);
		strncpy(MYSQL_PORT_NUMBER, "3306", sizeof(MYSQL_PORT_NUMBER) - 1);
		strncpy(MYSQL_USERNAME, "root", sizeof(MYSQL_USERNAME) - 1);
		strncpy(MYSQL_PASSWORD, "root", sizeof(MYSQL_PASSWORD) - 1);
		strncpy(MYSQL_DBNAME, "database", sizeof(MYSQL_DBNAME) - 1);
	}
	else
	{
		strncpy(MYSQL_HOST, reader.Get("MySQL", "MYSQL_HOST", "127.0.0.1").c_str(), sizeof(MYSQL_HOST) - 1);
		strncpy(MYSQL_PORT_NUMBER, reader.Get("MySQL", "MYSQL_PORT_NUMBER", "3306").c_str(), sizeof(MYSQL_PORT_NUMBER) - 1);
		strncpy(MYSQL_USERNAME, reader.Get("MySQL", "MYSQL_USERNAME", "root").c_str(), sizeof(MYSQL_USERNAME) - 1);
		strncpy(MYSQL_PASSWORD, reader.Get("MySQL", "MYSQL_PASSWORD", "root").c_str(), sizeof(MYSQL_PASSWORD) - 1);
		strncpy(MYSQL_DBNAME, reader.Get("MySQL", "MYSQL_DBNAME", "database").c_str(), sizeof(MYSQL_DBNAME) - 1);
	}

	LoadConfig(); // 首次调用初始化一些资源
	isPageSrcLoadSuccess = false;
}

void cleanup()
{
	// 清理操作，这些内存在 LoadConfig() 中分配。
	free(HEADER_TEMPLATE_LOCATION);
	free(FOOTER_TEMPLATE_LOCATION);
	free(SERVER_URL);
	free(USER_AGENT);
	free(OAUTH2_APPID);
	free(OAUTH2_SECRET);
	free(CURL_PROXY_URL);
	free(APP_NAME);
	free(CARD_AD_BANNER_1_IMG);
	free(CARD_AD_BANNER_2_IMG);
	free(CARD_AD_BANNER_1_URL);
	free(CARD_AD_BANNER_2_URL);
	free(ADMIN_USER_NAME);
	free(ADMIN_PASSWORD);
	free(SECONDARY_TITLE);
	free(APP_KEYWORDS);
	free(APP_DESCRIPTION);
	free(FOOTER_TEXT);
	free(ANALYSIS_CODE);
	free(HOMEPAGE_NOTICE);
	free(DISCUSSION_PAGE_CONTENT);
	free(DISCUSSION_PAGE_CODE);
	free(SITE_MAINTENANCE);

	if (dbConnError != NULL)
	{
		free(dbConnError);
		dbConnError = NULL;
	}

	mysql_close(&db);
	curl_global_cleanup();
}