#include "headers.h"
#include "Encrypt.h"
#include "General.h"
#include "StringHelper.h"
#include "AES.h"

static uint8_t key[] = "这里这里这里这里这里这里这里"; //加密密钥
static uint8_t iv[] = "改掉改掉改掉改掉改掉改掉改掉"; //初始化向量

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

	// 第一步加密后，第二步进行base64编码并去掉编码后字符中的等于号
	base64_encode(encryptedBytes.data(), out, encryptedBytes.size());
	for (int i = 0; *(out + i) != '\0'; i++)
	{
		if (*(out + i) == '=') {
			*(out + i) = '\0';
			break;
		}
	}

	int newlen = 0;
	char *urlenc = url_encode(out, (int)strlen(out), &newlen); // 第三步，URL编码
	memset(out, 0, newlen + 1);
	strncpy(out, urlenc, newlen);

	for(int i = 0; i < newlen; i++) // 第四步，进行URL安全化
	{
		if (*(out + i) == '%')
			*(out + i) = '_';
	}
	free(urlenc);
}

void DeCodeStr(char *pCode)
{
	int pCodelen = strlen(pCode);
	for (int i = 0; i < pCodelen; i++)
	{
		if (*(pCode + i) == '_')
			*(pCode + i) = '%';
	}
	url_decode(pCode, pCodelen);

	pCodelen = strlen(pCode);
	int mod_len = pCodelen % 4;
	for (; mod_len > 0; mod_len--)
	{
		strcat(pCode, "=");
	}

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