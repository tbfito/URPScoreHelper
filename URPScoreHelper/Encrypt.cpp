#include "stdafx.h"
#include "Encrypt.h"
#include "General.h"
#include "StringHelper.h"
#include "AES.h"

//长度有限，注意内存分配
void EnCodeStr(char *str, char *out)
{
	uint8_t *encrypted = (uint8_t *)malloc(4096);
	memset(encrypted, 0, 4096);
	AES_ECB_encrypt((const uint8_t *)str, (const uint8_t *)"e360b63e-4b61-4171-9531-8165a1ac87ac", encrypted, strlen(str));
	Ascii2Hex((char *)encrypted, out);
	free(encrypted);
}

void DeCodeStr(char *pCode)
{
	char *strCode = (char *)malloc(4096);
	memset(strCode, 0, 4096);
	Hex2Ascii(pCode, strCode);
	uint8_t *decrypted = (uint8_t *)malloc(4096);
	memset(decrypted, 0, 4096);
	AES_ECB_decrypt((const uint8_t *)strCode, (const uint8_t *)"e360b63e-4b61-4171-9531-8165a1ac87ac", decrypted, 4096);
	strcpy(pCode, (const char *)decrypted);
	free(decrypted);
	free(strCode);
}