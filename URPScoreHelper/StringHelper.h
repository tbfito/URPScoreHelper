#pragma once

#ifndef __STRINGHELPER_H__

	#define __STRINGHELPER_H__

	char * left(char *dst, char *src, int n);
	char * mid(char *dst, char *src, int n, int m);
	char * right(char *dst, char *src, int n);
	void replace_string(char * source_str, const char * targ_str, const char *val);
	char * url_encode(char const *s, int len, int *new_length);
	int url_decode(char *str, int len);
	void get_time(char *lpszTime);
	int split(char dst[][128], char* str, const char* spliter);
	std::string _URLFIND(const std::string & url, const char *name);
	std::string _GET(const std::string & get, const char *name);
	std::string _POST(const std::string & post, const char *name);
	std::string _COOKIE(const std::string & cookie, const char *name);

#endif