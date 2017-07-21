#include "stdafx.h"
#include "General.h"

char JSESSIONID[256] = {0};

const char* GLOBAL_HEADER = "X-Powered-By: iEdon-URPScoreHelper\r\n\
Content-Type: text/html; charset=utf-8\r\n\
Cache-Control: no-cache\r\n\
Pragma: no-cache\r\n\
Expires: -1\r\n\r\n";

// HTMLS
const char *BEFORE_TEMPLATE = u8"<div id=\"list_page\" style=\"background-color:transparent !important\"><table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"titleTop2\" style=\"background-color:transparent !important;border:none\"><tbody><tr><td class=\"pageAlign\"><table cellpadding=\"0\" width=\"100%\" class=\"displayTag\" cellspacing=\"1\" border=\"0\" id=\"user\"><thead><tr><th align=\"center\" width=\"30%\" class=\"sortable\">����</th><th align=\"center\" width=\"10%\" class=\"sortable\">�ɼ�</th><th align=\"center\" width=\"10%\" class=\"sortable\">����</th><th align=\"center\" width=\"10%\" class=\"sortable\">���</th><th align=\"center\" width=\"10%\" class=\"sortable\">���</th><th align=\"center\" width=\"10%\" class=\"sortable\">����</th><th align=\"center\" width=\"10%\" class=\"sortable\">ѧ��</th><th align=\"center\" width=\"10%\" class=\"sortable\">����</th></tr></thead><tbody>";
const char *AFTER_TEMPLATE = u8"</tbody></table></td></tr></tbody></table></div>";
const char *SCORE_TEMPLATE = u8"<tr class=\"even\" onmouseout=\"this.className='even';\" onmouseover=\"this.className='evenfocus';\"><td align=\"center\" style=\"%s\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%.1f</td></tr>";
const char *BEFORE_TEMPLATE_BY_PLAN = u8"<div id=\"list_page\" style=\"background-color:transparent !important\"><table width=\"100%\"border=\"0\"cellpadding=\"0\"cellspacing=\"0\"class=\"titleTop2\"style=\"background-color:transparent !important;border:none\"><tbody><tr><td class=\"pageAlign\"><table cellpadding=\"0\"width=\"100%\"class=\"displayTag\"cellspacing=\"1\"border=\"0\"id=\"user\"><thead><tr><th align=\"center\"width=\"45%\"class=\"sortable\">����</th><th align=\"center\"width=\"10%\"class=\"sortable\">���</th><th align=\"center\"width=\"10%\"class=\"sortable\">�ɼ�</th><th align=\"center\"width=\"10%\"class=\"sortable\">ѧ��</th><th align=\"center\"width=\"10%\"class=\"sortable\">����</th><th align=\"center\"width=\"15%\"class=\"sortable\">��ע</th></tr></thead><tbody>";
const char *SCORE_TEMPLATE_BY_PLAN = u8"<tr class=\"even\"onmouseout=\"this.className='even';\"onmouseover=\"this.className='evenfocus';\"><td align=\"center\"style=\"%s\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%.2f</td><td align=\"center\">%s</td></tr>";
const char *QUICK_SCORE = u8"<div class=\"weui-cell\"><div class=\"weui-cell__bd\"><p>%s</p></div><div class=\"weui-cell__ft\">%s</div></div>";
const char *OAUTH2_LOGIN_HTML = u8"<a title=\"QQ���ٵ�¼\" class=\"weui-btn weui-btn_default col-50\" href=\"OAuth2.fcgi\">QQ���ٵ�¼</a>";
const char *QUICKQUERY_HTML = u8"<div class=\"quickquery\"><a class=\"weui-btn weui-btn_warn\" href=\"QuickQuery.fcgi\">���ܿ��ٲ�ѯ��� &gt;&gt;</a></div>";
const char *ASSOC_LINK_HTML = u8"<a href=\"index.fcgi?act=requestAssoc\">��QQ�ʺ�</a>";
const char *RLS_ASSOC_LINK_HTML = u8"<span style=\"color:rgb(0,255,90)\">QQ�Ѱ�</span> | <a href=\"javascript:void(0);\" onclick=\"releaseAssoc('%s');\">�����</a>";

// ����ҳ������ó�ʼ��
std::string ERROR_HTML;
char *SERVER_URL = NULL;
char *USER_AGENT = NULL;
int CURL_TIMEOUT = 10;
char *OAUTH2_APPID = NULL;
char *OAUTH2_SECRET = NULL;
bool CURL_USE_PROXY = false;
char *CURL_PROXY_URL = NULL;
char *APP_NAME = NULL;

/*
�������ҳ��
@Params: char* error message
@Return: none
*/
void Error(const char *p_ErrMsg)
{
	std::string output = strformat(ERROR_HTML.c_str(), p_ErrMsg);
	std::cout << GLOBAL_HEADER << output.c_str();
}

/*
BASE64 ����
@Params: source bin, in buffer, in length
@Return char* base64 code
*/
const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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


/*
���ı��ļ������ڴ�
@Params: const char* file path
@Return: std::string filedata
*/
std::string ReadTextFileToMem(const char *lpszLocalPath)
{
	std::string fdata;
	FILE *m_file = fopen(lpszLocalPath, "rb");
	if (m_file == NULL)
	{
		return fdata;
	}
	fseek(m_file, 0, SEEK_END); // �Ƶ�β
	int m_file_length = ftell(m_file); // ȡ�ó���
	fseek(m_file, 0, SEEK_SET); // �Ƶ���
	char *m_lpszfdata = (char *)malloc(m_file_length + 1);
	ZeroMemory(m_lpszfdata, m_file_length + 1);
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
���ɼ�ת��Ϊ����
@Params: float cj
@Return: float jidian point
*/
float cj2jd(float cj)
{
	if (cj <= 59)
		return 0.0;
	return ((((int)cj % 60) / 10.0) + 1.0);
}

/*
ʵ�� std::string �ĸ�ʽ������
@Params: const char *format
@Return: std::string
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

std::string getAppURL()
{
	std::string text;
	if (CGI_HTTPS != NULL && strcmp(CGI_HTTPS, "") != 0
		&& strcmp(CGI_HTTPS, "off") != 0
		&& strcmp(CGI_HTTPS, "OFF") != 0
		&& strcmp(CGI_HTTPS, "0") != 0)
	{
		text.append("https://");
	}
	else
	{
		text.append("http://");
	}
	text.append(CGI_HTTP_HOST);
	return text;
}