#include "headers.h"
#include "gbkutf8.h"

int charset_convert(const char *from_charset, const char *to_charset, char *in_buf, size_t in_left, char *out_buf, size_t out_left) {
	memset(out_buf, 0, out_left);
	iconv_t icd;
	char *pin = in_buf;
	char *pout = out_buf;
	size_t out_len = out_left;
	if ((iconv_t)-1 == (icd = iconv_open(to_charset, from_charset))) {
		return -1;
	}
	if ((size_t)-1 == iconv(icd, &pin, &in_left, &pout, &out_left)) {
		iconv_close(icd);
		return -1;
	}
	out_buf[out_len - out_left] = 0;
	iconv_close(icd);
	int ret_len = (int)out_len - out_left;
	return ret_len;
}

int gbk_to_utf8(const char *from, unsigned int from_len, char **to, unsigned int *to_len)
{
	return charset_convert("gbk", "utf-8", (char *)from, from_len, *to, *to_len);
}

int utf8_to_gbk(const char *from, unsigned int from_len, char **to, unsigned int *to_len)
{
	return charset_convert("utf-8", "gbk", (char *)from, from_len, *to, *to_len);
}