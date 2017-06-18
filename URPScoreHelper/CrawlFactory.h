#pragma once

bool CrawlRequest(const char *p_rq, char *p_lpvBuffer, int p_iLength, int *p_iTotalRead, bool no_error_page = false);
bool InitSocketLibrary();