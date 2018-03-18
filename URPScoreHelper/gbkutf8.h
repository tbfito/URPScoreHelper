#pragma once

#ifndef __GBKUTF8_H__

	#define __GBKUTF8_H__

	int charset_convert(const char *from_charset, const char *to_charset, char *in_buf, size_t in_left, char *out_buf, size_t out_left);
	int gbk_to_utf8(const char *from, unsigned int from_len, char **to, unsigned int *to_len);
	int utf8_to_gbk(const char *from, unsigned int from_len, char **to, unsigned int *to_len);

#endif