/*
******************************************
********** iEdon URPScoreHelper **********
**********  C MVC - Controller  **********
**********  Copyright (C) iEdon **********
******************************************
****  Project encoding must be UTF-8  ****
******************************************
*/

#include "stdafx.h"
#include "main.h"
#include "General.h"
#include "gbkutf8.h"
#include "INIReader.h"

int main(int argc, const char *argv[])
{
	strcat(GLOBAL_HEADER, "Content-Type: text/html; charset=utf-8\r\nX-Powered-By: ");
	strcat(GLOBAL_HEADER, SOFTWARE_NAME);
	strcat(GLOBAL_HEADER, "\r\n\r\n");

	str_normalize_init();
	FCGX_Init();
	curl_global_init(CURL_GLOBAL_ALL);
	mysql_init(&db);

	char *root = (char *)malloc(MAX_PATH);
	memset(root, 0, MAX_PATH);
	getcwd(root, MAX_PATH);
	char *pStr = strstr(root, "\\");
	bool isUnixBasedPath = (pStr == NULL);
	if (isUnixBasedPath)
	{
		strcat(root, "/Database.ini");
	}
	else
	{
		strcat(root, "\\Database.ini");
	}

	INIReader reader(root);
	if (reader.ParseError() != 0) {
		strcpy(MYSQL_HOST, "127.0.0.1");
		strcpy(MYSQL_PORT_NUMBER, "3306");
		strcpy(MYSQL_USERNAME, "root");
		strcpy(MYSQL_PASSWORD, "root");
		strcpy(MYSQL_DBNAME, "database");
	}
	else
	{
		strcpy(MYSQL_HOST, reader.Get("MySQL", "MYSQL_HOST", "127.0.0.1").c_str());
		strcpy(MYSQL_PORT_NUMBER, reader.Get("MySQL", "MYSQL_PORT_NUMBER", "3306").c_str());
		strcpy(MYSQL_USERNAME, reader.Get("MySQL", "MYSQL_USERNAME", "root").c_str());
		strcpy(MYSQL_PASSWORD, reader.Get("MySQL", "MYSQL_PASSWORD", "root").c_str());
		strcpy(MYSQL_DBNAME, reader.Get("MySQL", "MYSQL_DBNAME", "database").c_str());
	}
	free(root);

	LoadConfig();
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
	fastcgi_app_intro();
	printf("%s\n%s\n\n%s\n", SOFTWARE_NAME, SOFTWARE_COPYRIGHT, "\tOptions: [-p (localhost):port_number]");

	// cleanup operations
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
	if (dbConnError != NULL)
	{
		free(dbConnError);
		dbConnError = NULL;
	}

	mysql_close(&db);
	curl_global_cleanup();
	return 0;
}