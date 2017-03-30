#pragma once

extern bool CrawlRequest(const char *p_rq, char *p_lpvBuffer, int p_iLength, int *p_iTotalRead);
extern bool InitSocketLibrary();