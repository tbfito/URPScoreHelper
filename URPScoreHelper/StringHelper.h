#pragma once
char * left(char *dst, char *src, int n);
char * mid(char *dst, char *src, int n, int m);
char * right(char *dst, char *src, int n);
void replace_string(char * source_str, const char * targ_str, const char *val);
char * url_encode(char const *s, int len, int *new_length);
int url_decode(char *str, int len);
void get_time(char *lpszTime);
int split(char dst[][128], char* str, const char* spliter);
int Hex2Ascii(char* hex, unsigned char* ascii);
int Ascii2Hex(char* ascii, char* hex);
static std::string _URLFIND(std::string & url, const char *name);
std::string _GET(std::string & get, const char *name);
std::string _POST(std::string & post, const char *name);
std::string _COOKIE(std::string & cookie, const char *name);