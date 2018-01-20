#include "headers.h"
#include "StringHelper.h"

// 取文本左边
char *left(char *dst, char *src, int n)
{
	char *p = src;
	char *q = dst;
	int	len = strlen(src);
	if (n > len)
		n = len;
	while (n--)
		*(q++) = *(p++);
	*(q++) = '\0';                                  /*有必要吗？很有必要*/
	return(dst);
}

// 取文本中间
char *mid(char *dst, char *src, int n, int m)        /*n为长度，m为位置*/
{
	char	*p = src;
	char	*q = dst;
	int	len = strlen(src);
	if (n > len)
		n = len - m;                            /*从第m个到最后*/
	if (m < 0)
		m = 0;                                  /*从第一个开始*/
	if (m > len)
		return(NULL);
	p += m;
	while (n--)
		*(q++) = *(p++);
	*(q++) = '\0';                                  // 有必要吗？很有必要
	return(dst);
}

// 取文本右边
char *right(char *dst, char *src, int n)
{
	char	*p = src;
	char	*q = dst;
	int	len = strlen(src);
	if (n > len)
		n = len;
	p += (len - n);					// 从右边第n个字符开始，到0结束
	while ((*(q++) = *(p++)));		// 遍历 src 并赋值给 dst
	return(dst);
}

// 只允许将长串替换为短串
void replace_string(char * source_str, const char * search, const char *replace_with)
{
	size_t dst_size = strlen(source_str);
	char * replace_buf = (char *)malloc(dst_size);
	if (replace_buf)
	{
		replace_buf[0] = 0;
		char * p = (char *)source_str;
		char * pos = NULL;
		while ((pos = strstr(p, search)) != NULL)
		{
			size_t n = (size_t)(pos - p);
			strncat(replace_buf, p, n > dst_size ? dst_size : n);
			strncat(replace_buf, replace_with, dst_size - strlen(replace_buf) - 1);
			p = pos + strlen(search);
		}
		snprintf(source_str, dst_size, "%s%s", replace_buf, p);
		free(replace_buf);
	}
}

static unsigned char hexchars[] = "0123456789ABCDEF";
static int htoi(char *s)
{
	int value;
	int c;

	c = ((unsigned char *)s)[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = ((unsigned char *)s)[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}

char * url_encode(char const *s, int len, int *new_length)
{
	unsigned char c, *to, *start;
	unsigned char const *from, *end;

	from = (unsigned char *)s;
	end = (unsigned char *)s + len;
	start = to = (unsigned char *)calloc(1, 3 * len + 1);

	while (from < end)
	{
		c = *from++;

		if (c == ' ')
		{
			*to++ = '+';
		}
		else if ((c < '0' && c != '-' && c != '.') ||
			(c < 'A' && c > '9') ||
			(c > 'Z' && c < 'a' && c != '_') ||
			(c > 'z'))
		{
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
		}
		else
		{
			*to++ = c;
		}
	}
	*to = 0;
	if (new_length)
	{
		*new_length = to - start;
	}
	return (char *)start;
}

int url_decode(char *str, int len)
{
	char *dest = str;
	char *data = str;

	while (len--)
	{
		if (*data == '+')
		{
			*dest = ' ';
		}
		else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2)))
		{
			*dest = (char)htoi(data + 1);
			data += 2;
			len -= 2;
		}
		else
		{
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}

void get_time(char *lpszTime)
{
	time_t now;
	struct tm *tm_now;
	time(&now);
	tm_now = localtime(&now);
	sprintf(lpszTime , "%d-%02d-%02d %02d:%02d:%02d\n",
		tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
}

// 将str字符以spliter分割,存于dst中，并返回子字符串数量
int split(char dst[][128], char* str, const char* spliter)
{
	int n = 0;
	char *result = NULL;
	result = strtok(str, spliter);
	while (result != NULL)
	{
		strncpy(dst[n++], result, 127);
		result = strtok(NULL, spliter);
	}
	return n;
}

int Hex2Ascii(char* hex, unsigned char* ascii)
{
	int len = strlen(hex), tlen, i, cnt;

	for (i = 0, cnt = 0, tlen = 0; i<len; i++)
	{
		char c = toupper(hex[i]);

		if ((c >= '0'&& c <= '9') || (c >= 'A'&& c <= 'F'))
		{
			unsigned char t = (c >= 'A') ? c - 'A' + 10 : c - '0';

			if (cnt)
				ascii[tlen++] += t, cnt = 0;
			else
				ascii[tlen] = t << 4, cnt = 1;
		}
	}

	return tlen;
}

int Ascii2Hex(char* ascii, char* hex)
{
	int i, len = strlen(ascii);

	for (i = 0; i<len; i++)
	{
		hex[i * 2] = hexchars[((unsigned char)ascii[i]) >> 4];
		hex[i * 2 + 1] = hexchars[((unsigned char)ascii[i]) & 0xf];
	}

	hex[len * 2] = '\0';
	return len * 2;
}

// 获取 GET 中的内容
std::string _GET(const std::string & get, const char *name)
{
	return _URLFIND(get, name);
}

// 获取 POST 中的内容
std::string _POST(const std::string & post, const char *name)
{
	return _URLFIND(post, name);
}

std::string _URLFIND(const std::string & url, const char *name)
{
	std::string var(name);
	var.append("=");
	size_t len = var.length();
	std::string ret;

	size_t pos1 = url.find(var);
	size_t pos2 = url.find("&", pos1 + len);
	if (pos1 == std::string::npos)
	{
		return ret;
	}
	if (pos2 == std::string::npos)
	{
		ret = url.substr(pos1 + len, url.length() - pos1 - len);
	}
	else
	{
		ret = url.substr(pos1 + len, pos2 - pos1 - len);
	}
	return ret;
}

// 获取 COOKIE 中的内容
std::string _COOKIE(const std::string & cookie, const char *name)
{
	std::string var(name);
	var.append("=");
	size_t len = var.length();
	std::string ret;

	size_t pos1 = cookie.find(var);
	size_t pos2 = cookie.find(";", pos1 + len);
	if (pos1 == std::string::npos)
	{
		return ret;
	}
	if (pos2 == std::string::npos)
	{
		ret = cookie.substr(pos1 + len, cookie.length() - pos1 - len);
	}
	else
	{
		ret = cookie.substr(pos1 + len, pos2 - pos1 - len);
	}
	return ret;
}
