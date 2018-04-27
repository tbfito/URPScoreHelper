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

// 注意串长容量
void replace_string(char * source_str, const char * search, const char *replace_with)
{
	std::string data(source_str);
	std::string toSearch(search);
	std::string replaceStr(replace_with);
	findAndReplaceAll(data, toSearch, replaceStr);
	strncpy(source_str, data.c_str(), data.size());
	source_str[data.size() + 1] = '\0';
}

void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr)
{
	// Get the first occurrence
	size_t pos = data.find(toSearch);

	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos = data.find(toSearch, pos + toSearch.size());
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

//注意释放内存
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

/*
BASE64 编码
*/
static const char *base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char * base64_encode(const unsigned char * bindata, char * base64, int binlength)
{
	int i, j;
	unsigned char current;

	for (i = 0, j = 0; i < binlength; i += 3)
	{
		current = (bindata[i] >> 2);
		current &= (unsigned char)0x3F;
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)(bindata[i] << 4)) & ((unsigned char)0x30);
		if (i + 1 >= binlength)
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}
		current |= ((unsigned char)(bindata[i + 1] >> 4)) & ((unsigned char)0x0F);
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)(bindata[i + 1] << 2)) & ((unsigned char)0x3C);
		if (i + 2 >= binlength)
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			break;
		}
		current |= ((unsigned char)(bindata[i + 2] >> 6)) & ((unsigned char)0x03);
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)bindata[i + 2]) & ((unsigned char)0x3F);
		base64[j++] = base64char[(int)current];
	}
	base64[j] = '\0';
	return base64;
}

static const unsigned char g_pMap[256] =
{
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
	52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
	255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
	7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
	19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
	255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
	37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
	49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255
};
bool base64_decode(const char *strIn, unsigned char *pOut, size_t *uOutLen)
{
	size_t t, x, y, z;
	unsigned char c;
	size_t g = 3;

	//ASSERT((pIn != NULL) && (uInLen != 0) && (pOut != NULL) && (uOutLen != NULL));
	size_t len = strlen(strIn);
	for (x = y = z = t = 0; x < len; x++)
	{
		c = g_pMap[(const unsigned char)strIn[x]];
		if (c == 255) continue;
		if (c == 254) { c = 0; g--; }

		t = (t << 6) | c;

		if (++y == 4)
		{
			if ((z + g) > *uOutLen) { return false; } // Buffer overflow
			pOut[z++] = (unsigned char)((t >> 16) & 255);
			if (g > 1) pOut[z++] = (unsigned char)((t >> 8) & 255);
			if (g > 2) pOut[z++] = (unsigned char)(t & 255);
			y = t = 0;
		}
	}

	*uOutLen = z;
	return true;
}

/*
将文本文件读入内存
*/
std::string ReadTextFileToMem(const char *lpszLocalPath)
{
	std::string fdata;
	FILE *m_file = fopen(lpszLocalPath, "rb");
	if (m_file == NULL)
	{
		return fdata;
	}
	fseek(m_file, 0, SEEK_END); // 移到尾
	int m_file_length = ftell(m_file); // 取得长度
	fseek(m_file, 0, SEEK_SET); // 移到首
	char *m_lpszfdata = (char *)malloc(m_file_length + 1);
	memset(m_lpszfdata, 0, m_file_length + 1);
	if (fread(m_lpszfdata, m_file_length, 1, m_file) != 1)
	{
		fclose(m_file);
		return fdata;
	}
	fclose(m_file);
	fdata.append(m_lpszfdata);
	free(m_lpszfdata);
	return fdata;
}

/*
实现 std::string 的格式化功能
*/
std::string strformat(const char *format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);
	int needed = vsnprintf(NULL, 0, format, arg_list) + 1;
	va_end(arg_list);
	char *p = (char *)malloc(needed);
	va_start(arg_list, format);
	vsnprintf(p, needed, format, arg_list);
	va_end(arg_list);
	std::string str(p);
	free(p);
	return str;
}
