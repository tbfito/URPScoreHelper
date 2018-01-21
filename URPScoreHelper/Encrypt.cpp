#include "headers.h"
#include "Encrypt.h"
#include "General.h"
#include "StringHelper.h"
#include "AES.h"
#include <vector>

static uint8_t key[] = "e360b63e-4b61-4171-9531-8165a1af"; //加密密钥
static uint8_t iv[] = "c986fcfa-5c41-4fd6-ad8f-81000661"; //初始化向量

//长度有限，注意内存分配
void EnCodeStr(const char *str, char *out)
{
	std::string src(str);

	std::vector<uint8_t> encryptedBytes;

	for (auto it = src.begin(); it != src.end(); )
	{
		uint8_t plain[16] = { 0 }, enc[16] = { 0 };
		size_t i = 0;

		for (; it != src.end() && i < 16; ++i, ++it)
		{
			plain[i] = *it;
		}

		AES_CBC_encrypt_buffer(enc, plain, 16, key, iv);
		encryptedBytes.insert(encryptedBytes.end(), enc, enc + 16);
	}

	base64_encode(encryptedBytes.data(), out, encryptedBytes.size());
}

void DeCodeStr(char *pCode)
{
	size_t outlen = 4096;
	uint8_t *strCode = (uint8_t *)malloc(outlen);
	memset(strCode, 0, outlen);

	if (!base64_decode(pCode, strCode, &outlen)) // Overflow
	{
		free(strCode);
		return;
	}

	std::vector<uint8_t> decrypted;
	std::vector<uint8_t> encryptedBytes;
	for (size_t i = 0; i < outlen; i++)
	{
		encryptedBytes.insert(encryptedBytes.end(), *(strCode + i));
	}
	free(strCode);

	if (encryptedBytes.size() % 16 != 0) // 密文不是16的倍数，直接返回。继续执行会崩溃
	{
		return;
	}
	for (auto it = encryptedBytes.begin(); it != encryptedBytes.end(); it = std::next(it, 16))
	{
		uint8_t tmp[16] = {};
		AES_CBC_decrypt_buffer(tmp, (uint8_t *)&(*it), 16, key, iv);
		decrypted.insert(decrypted.end(), tmp, tmp + 16);
	}

	std::string res(decrypted.begin(), decrypted.end());

	// 因为解码后的字符串在范围控制内啊 TAT
	size_t srclen = res.length() + 1;
	memset(pCode, 0, srclen);
	strncpy(pCode, res.c_str(), srclen - 1);
}