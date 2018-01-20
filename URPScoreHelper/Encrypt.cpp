#include "headers.h"
#include "Encrypt.h"
#include "General.h"
#include "StringHelper.h"
#include "AES.h"
#include <vector>

static uint8 key[] = "e360b63e-4b61-4171-9531-8165a1ac87ac";

//长度有限，注意内存分配
void EnCodeStr(char *str, char *out)
{
	aes_context ctx = {};
	aes_set_key(&ctx, key, 128);

	std::string src(str);

	// will hold out encrypted message
	std::vector<uint8> encryptedBytes;

	// encrypt the data.
	for (auto it = src.begin(); it != src.end(); )
	{
		uint8 plain[16] = { 0 }, enc[16] = { 0 };
		size_t i = 0;
		for (; it != src.end() && i < 16; ++i, ++it)
			plain[i] = *it;
		aes_encrypt(&ctx, plain, enc);
		encryptedBytes.insert(encryptedBytes.end(), enc, enc + 16);
	}

	Ascii2Hex((char *)std::string(encryptedBytes.begin(), encryptedBytes.end()).c_str(), out);
}

void DeCodeStr(char *pCode)
{
	aes_context ctx = {};
	aes_set_key(&ctx, key, 128);

	uint8 *strCode = (uint8 *)malloc(4096);
	memset(strCode, 0, 4096);
	Hex2Ascii(pCode, strCode);

	std::vector<uint8> decrypted;
	std::string encryptedBytes((char *)strCode);
	free(strCode);

	if (encryptedBytes.length() % 16 != 0) // 密文不是16的倍数，直接返回。继续执行会崩溃
	{
		return;
	}
	for (auto it = encryptedBytes.begin(); it != encryptedBytes.end(); it = std::next(it, 16))
	{
		uint8 tmp[16] = {};
		aes_decrypt(&ctx, (uint8 *)&(*it), tmp);
		decrypted.insert(decrypted.end(), tmp, tmp + 16);
	}

	// rebuild string from data
	std::string res(decrypted.begin(), decrypted.end());

	// 因为解码后的字符串在范围控制内啊 TAT
	size_t srclen = res.length() + 1;
	memset(pCode, 0, srclen);
	strncpy(pCode, res.c_str(), srclen - 1);
}