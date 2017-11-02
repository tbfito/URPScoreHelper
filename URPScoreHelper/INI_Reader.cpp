#include "stdafx.h"
#include "INI_Reader.h"

unsigned int get_profile_string(char* chAppName, char* chKeyName, char* chDefault, char* chReturnedString, int nSize, char* chFileName)
{
	FILE  *fp = NULL;
	unsigned int ret = 0;
	char *tempDefValue = NULL;
	int  AppNameLen = 0, KeyNameLen = 0;

	if (!chReturnedString || nSize <= 1)
	{
		if (nSize == 1)
			chReturnedString = '\0';
		return 0;
	}

	memset(chReturnedString, 0, nSize * sizeof(char));


	if (chDefault)
	{
		char *p = chDefault + strlen(chDefault) - 1;
		while (p > chDefault && *p == ' ')
			p--;
		if (p >= chDefault)
		{
			int len = (int)(p - chDefault) + 1;
			tempDefValue = (char*)malloc(len + 1);
			strncpy(tempDefValue, chDefault, len);
			tempDefValue[len] = '\0';
			chDefault = tempDefValue;
		}
	}
	else
		chDefault = "\0";


	if (chAppName)
	{
		char *p;
		while (isspace((unsigned char)*chAppName)) chAppName++;
		if (*chAppName)
			p = chAppName + strlen(chAppName) - 1;
		else
			p = chAppName;
		while ((p > chAppName) && isspace((unsigned char)*p)) p--;
		AppNameLen = (int)(p - chAppName + 1);
	}


	if (chKeyName)
	{
		char *p;
		while (isspace((unsigned char)*chKeyName)) chKeyName++;
		if (*chKeyName)
			p = chKeyName + strlen(chKeyName) - 1;
		else
			p = chKeyName;
		while ((p > chKeyName) && isspace((unsigned char)*p)) p--;
		KeyNameLen = (int)(p - chKeyName + 1);
	}


	if (!chFileName)
	{
		if (chAppName)
		{
			int len = (int)strlen(chDefault);
			if (nSize <= len)
			{
				strncpy(chReturnedString, chDefault, nSize - 1);
				chReturnedString[nSize - 1] = '\0';
				ret = nSize - 1;
			}
			else
			{
				strncpy(chReturnedString, chDefault, len);
				ret = len;
			}
		}
		else
		{
			ret = 0;
			*chReturnedString = '\0';
		}
		if (tempDefValue) free(tempDefValue);
		return ret;
	}
	if (!(fp = fopen(chFileName, "r+")))
	{
		if (chAppName)
		{
			int len = (int)strlen(chDefault);
			if (nSize <= len)
			{
				strncpy(chReturnedString, chDefault, nSize - 1);
				chReturnedString[nSize - 1] = '\0';
				ret = nSize - 1;
			}
			else
			{
				strncpy(chReturnedString, chDefault, len);
				ret = len;
			}
		}
		else
		{
			ret = 0;
			*chReturnedString = '\0';
		}
		if (tempDefValue) free(tempDefValue);
		return ret;
	}



	bool bIsFindAppName = false, bIsFindKeyName = false;;
	char *ReadBuffer = NULL;
	char *szLineStart = NULL, *szLineEnd = NULL, *szEnd = NULL;
	char *next_line = NULL, *szValueStart = NULL;

	char tempAppName[1024] = "";
	char tempKeyName[1024] = "";
	char *TempValue = NULL;


	char * CopySectionName = (char*)malloc(nSize);
	char * CopyKeyName = (char*)malloc(nSize);
	if (!CopyKeyName || !CopySectionName)
	{
		fclose(fp);
		if (tempDefValue) free(tempDefValue);

		return 0;
	}
	char *p, *q;
	int  len = 0;
	int plen = nSize - 1, qlen = nSize - 1;
	bool bSectionCopyOver = false, bEntryCopyOver = false;


	memset(CopySectionName, 0, nSize);
	memset(CopyKeyName, 0, nSize);


	p = CopySectionName;
	q = CopyKeyName;


	fseek(fp, 0, SEEK_END);
	long nFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);


	if (nFileSize == INVALID_FILE_SIZE || nFileSize == 0)
	{
		int len = (int)strlen(chDefault);
		if (nSize <= len)
		{
			strncpy(chReturnedString, chDefault, nSize - 1);
			chReturnedString[nSize - 1] = '\0';
			ret = nSize - 1;
		}
		else
		{
			strncpy(chReturnedString, chDefault, len);
			ret = len;
		}


		fclose(fp);
		if (tempDefValue) free(tempDefValue);

		return ret;
	}
	else
	{
		ReadBuffer = (char*)malloc(nFileSize + 1);
		if (!ReadBuffer)
		{
			fclose(fp);
			if (tempDefValue) free(tempDefValue);
			return ret;
		}
		memset(ReadBuffer, 0, nFileSize + 1);
		if (!fread(ReadBuffer, sizeof(char), nFileSize, fp))
		{
			fclose(fp);
			free(ReadBuffer);
			if (tempDefValue) free(tempDefValue);
			return ret;
		}
	}

	len = (int)strlen(ReadBuffer);
	next_line = ReadBuffer;
	szEnd = ReadBuffer + len;


	while (next_line < szEnd)                                    //  analysis ReadBuffer
	{
		szLineStart = next_line;
		next_line = (char*)memchr(szLineStart, '\n', szEnd - szLineStart);
		if (!next_line)
			next_line = (char*)memchr(szLineStart, '\r', szEnd - szLineStart);
		if (!next_line)
			next_line = szEnd;
		else
			next_line++;


		szLineEnd = next_line;


		while (szLineStart < szLineEnd && isspace((unsigned char)*szLineStart))  szLineStart++;
		while ((szLineEnd > szLineStart) && isspace((unsigned char)szLineEnd[-1])) szLineEnd--;


		if (szLineStart >= szLineEnd) continue;


		if (*szLineStart == '[')
			/*section start*/
		{
			if (bIsFindAppName)
			{
				bEntryCopyOver = true;
			}
			const char *szAppNameEnd;
			if ((szAppNameEnd = (char*)memchr(szLineStart, ']', szLineEnd - szLineStart)))
			{
				szLineStart++;
				len = (int)(szAppNameEnd - szLineStart);
				strncpy(tempAppName, szLineStart, len);
				tempAppName[len] = '\0';
				if (!bSectionCopyOver)                   /* Copy all SectionName to  CopySectionName buffer*/
				{
					int templen = (int)strlen(tempAppName) + 1;
					if (templen >= plen)
					{
						if (plen > 0)
						{
							strncpy(p, tempAppName, plen);
							p += plen - 1;
							*p++ = '\0';
						}
						*p = '\0';
						ret = nSize - 2;
						bSectionCopyOver = true;
					}
					else
					{
						strncpy(p, tempAppName, templen - 1);
						strncpy(p + templen - 1, "\t", 1);
					}

					p += templen;
					plen -= templen;
				}
				if (chAppName)
				{
					if (0 == _strnicmp(tempAppName, chAppName, AppNameLen)
						&& tempAppName[AppNameLen] == '\0')                 /* AppName is found */
						bIsFindAppName = true;
				}
				continue;
			}
		}


		len = (int)(szLineEnd - szLineStart);
		if ((szValueStart = (char*)memchr(szLineStart, '=', szLineEnd - szLineStart)) != NULL)
		{
			const char *szKeyNameEnd = szValueStart;
			while (szKeyNameEnd > szLineStart &&isspace((unsigned char)szKeyNameEnd[-1]))  szKeyNameEnd--;
			len = (int)(szKeyNameEnd - szLineStart);
			szValueStart++;
			while (szValueStart < szLineEnd && isspace((unsigned char)*szValueStart)) szValueStart++;


			if (len > 0 && bIsFindAppName)
			{
				strncpy(tempKeyName, szLineStart, len);
				tempKeyName[len] = '\0';
				if (!bEntryCopyOver)                  /* if AppName is found ,copy All KeyName to CopyKeyName buffer*/
				{
					int templen = (int)strlen(tempKeyName) + 1;
					if (templen >= qlen)
					{
						if (qlen > 0)
						{
							strncpy(q, tempKeyName, qlen);
							q += qlen - 1;
							*q++ = '\0';
						}
						*q = '\0';
						ret = nSize - 2;
						bEntryCopyOver = true;
					}
					else
					{
						strncpy(q, tempKeyName, templen - 1);
						strncpy(q + templen - 1, "\t", 1);
					}
					q += templen;
					qlen -= templen;
				}
				if (chKeyName)
				{
					if (0 == _strnicmp(tempKeyName, chKeyName, KeyNameLen) && tempKeyName[KeyNameLen] == '\0')    /* AppName and kyeName are found*/
					{
						bIsFindKeyName = true;
						len = (int)(szLineEnd - szValueStart);
						TempValue = (char*)malloc(len + 1);

						memcpy(TempValue, szValueStart, len * sizeof(char));
						TempValue[len] = '\0';
						break;

					}
				}
			}
		}
	}

	if (bIsFindKeyName)                     /* KeyName is found */
	{
		if (TempValue)
		{
			len = (int)strlen(TempValue);
			if (nSize>len)
			{
				strncpy(chReturnedString, TempValue, len);
				ret = len;
			}
			else
			{
				strncpy(chReturnedString, TempValue, nSize - 1);
				chReturnedString[nSize - 1] = '\0';
				ret = nSize - 1;
			}
		}
		else
			ret = 0;
	}
	else if (bIsFindAppName)             /* Section is found,but Entry is not found */
	{
		if (!chKeyName)
		{
			len = (int)strlen(CopyKeyName);
			strncpy(chReturnedString, CopyKeyName, len);
			ret = len;
		}
		else
		{
			len = (int)strlen(chDefault);
			if (nSize>len)
			{
				strncpy(chReturnedString, chDefault, len);
				ret = len;
			}
			else
			{
				strncpy(chReturnedString, chDefault, nSize - 1);
				chReturnedString[nSize - 1] = '\0';
				ret = nSize - 1;                           /* UnSure return nSize or nSize-1*/
			}
		}
	}
	else                                           /* Section is not found*/
	{
		if (!chAppName)
		{
			len = (int)strlen(CopySectionName);
			strncpy(chReturnedString, CopySectionName, len);
			ret = len;
		}
		else
		{
			len = (int)strlen(chDefault);
			if (nSize>len)
			{
				strncpy(chReturnedString, chDefault, len);
				ret = len;
			}
			else
			{
				strncpy(chReturnedString, chDefault, nSize - 1);
				chReturnedString[nSize - 1] = '\0';
				ret = nSize - 1;                           /* UnSure return nSize or nSize-1*/
			}
		}
	}

	if (tempDefValue)    free(tempDefValue);
	if (CopySectionName) free(CopySectionName);
	if (CopyKeyName)     free(CopyKeyName);
	if (ReadBuffer)      free(ReadBuffer);
	if (TempValue)       free(TempValue);

	fclose(fp);
	return ret;
}