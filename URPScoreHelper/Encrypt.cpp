#include "stdafx.h"
#include "Encrypt.h"
#include "General.h"
#include "StringHelper.h"

//str����Ϊ�ɴ�ӡ�ַ�0x20 -- 0x7F  
//str������'\0'��β,  
//����Ч���Ϻ�
// ���ܽ��Ϊ�ɴ�ӡ�ַ�0x20 -- 0x7F  
// ����������������ַ����������ı�
void EnCodeStr(char *str, char *out)
{
	int randOffset = rand() % 96;
	int ResChar;
	int i = 0;
	for (; str[i] != '\0'; i++)
	{
		ResChar = randOffset + str[i] + i * 33; //33����Ϊ�������ʵ�����  
		while (ResChar > 127)
		{
			ResChar -= 96;
		}
		str[i] = (char)ResChar;
	}
	str[i++] = (char)(randOffset + 32);
	str[i] = '\0';
	int len = strlen(str) * 2 + 1;
	char *tmp = (char *)malloc(len);
	memset(tmp, 0, len);
	Ascii2Hex(str, tmp);
	for (int x = 0; tmp[x]; x++)
		tmp[x] = tmp[x] ^ APP_NAME[x % strlen(APP_NAME)];
	Ascii2Hex(tmp, out);
	free(tmp);
}
//str����Ϊ�ɴ�ӡ�ַ� 0x20 -- 0x7F  
//str������'\0'��β,  
void DeCodeStr(char *pCode)
{
	int len = strlen(pCode);
	char *strCode = (char*)malloc(len);
	memset(strCode, 0, len);
	len = Hex2Ascii(pCode, strCode);
	strcpy(pCode, strCode);
	for (int x = 0; pCode[x]; x++)
		pCode[x] = pCode[x] ^ APP_NAME[x % strlen(APP_NAME)];
	memset(strCode, 0, len);
	Hex2Ascii(pCode, strCode);
	strcpy(pCode, strCode);
	free(strCode);
	int KeyIndex = 0;
	if (pCode[0] == '\0')  return;
	while (pCode[KeyIndex] != '\0')
	{
		KeyIndex++;
	}
	int Key = (int)(pCode[KeyIndex - 1]) - 32;
	int ResChar;
	int i = 0;
	for (; pCode[i] != '\0'; i++)
	{
		ResChar = pCode[i] - Key - i * 33;
		while (ResChar < 32)
		{
			ResChar += 96;
		}
		pCode[i] = (char)ResChar;
	}
	pCode[i - 1] = '\0';
}