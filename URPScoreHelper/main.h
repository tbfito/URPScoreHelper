#pragma once

extern void fastcgi_app_intro();
extern void LoadConfig();
extern FCGX_Request request;
extern bool isPageSrcLoadSuccess;
extern char *dbConnError;
void initialize();
void cleanup();
