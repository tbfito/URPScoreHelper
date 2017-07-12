#include "stdafx.h"
#include "General.h"

char JSESSIONID[256] = {0};

const char* GLOBAL_HEADER = "X-Powered-By: iEdon-URPScoreHelper\r\n\
Content-Type: text/html; charset=gb2312\r\n\
Cache-Control: no-cache\r\n\
Pragma: no-cache\r\n\
Expires: -1\r\n\r\n";

// 分数显示块
const char *BEFORE_TEMPLATE = "<div id=\"list_page\" style=\"background-color:transparent !important\"><table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"titleTop2\" style=\"background-color:transparent !important;border:none\"><tbody><tr><td class=\"pageAlign\"><table cellpadding=\"0\" width=\"100%\" class=\"displayTag\" cellspacing=\"1\" border=\"0\" id=\"user\"><thead><tr><th align=\"center\" width=\"30%\" class=\"sortable\">课程名称</th><th align=\"center\" width=\"10%\" class=\"sortable\">成绩</th><th align=\"center\" width=\"10%\" class=\"sortable\">均分</th><th align=\"center\" width=\"10%\" class=\"sortable\">最高</th><th align=\"center\" width=\"10%\" class=\"sortable\">最低</th><th align=\"center\" width=\"10%\" class=\"sortable\">名次</th><th align=\"center\" width=\"10%\" class=\"sortable\">学分</th><th align=\"center\" width=\"10%\" class=\"sortable\">绩点</th></tr></thead><tbody>";
const char *AFTER_TEMPLATE = "</tbody></table></td></tr></tbody></table></div>";
const char *SCORE_TEMPLATE = "<tr class=\"even\" onmouseout=\"this.className='even';\" onmouseover=\"this.className='evenfocus';\"><td align=\"center\" style=\"%s\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%.1f</td></tr>";
const char *QUICK_SCORE = "<div class=\"weui-cell\"><div class=\"weui-cell__bd\"><p>%s</p></div><div class=\"weui-cell__ft\">%s</div></div>";

// 错误页面和配置初始化
std::string ERROR_HTML;
char *SERVER_URL = NULL;
char *USER_AGENT = NULL;
int CURL_TIMEOUT = 10;
char *OAUTH2_APPID = NULL;
char *OAUTH2_SECRET = NULL;
bool CURL_USE_PROXY = false;
char *CURL_PROXY_URL = NULL;

/*
输出错误页面
@Params: char* error message
@Return: none
*/
void Error(const char *p_ErrMsg)
{
	std::string output = strformat(ERROR_HTML.c_str(), p_ErrMsg);
	std::cout << GLOBAL_HEADER << output.c_str();
}

/*
BASE64 编码
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
将文本文件读入内存
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
	fseek(m_file, 0, SEEK_END); // 移到尾
	int m_file_length = ftell(m_file); // 取得长度
	fseek(m_file, 0, SEEK_SET); // 移到首
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
将成绩转化为绩点
@Params: float cj
@Return: float jidian point
*/
float cj2jd(float cj)
{
	if (cj <= 59)
		return 0;
	return ((((int)cj % 60) / 10.0) + 1.0);
}

/*
实现 std::string 的格式化功能
@Params: const char *format
@Return: std::string
*/
std::string strformat(const char *format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);
	// SUSv2 version doesn't work for buf NULL/size 0, so try printing
	// into a small buffer that avoids the double-rendering and alloca path too...
	char short_buf[256];
	const size_t needed = vsnprintf(short_buf, 256, format, arg_list) + 1;
	if (needed <= 256)
	{
		std::string str(short_buf);
		return str;
	}

	// need more space...
	char* p = static_cast<char*>(alloca(needed));
	vsnprintf(p, needed, format, arg_list);
	std::string str(p);
	return str;
}