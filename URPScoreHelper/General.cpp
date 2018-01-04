#include "headers.h"
#include "General.h"

char JSESSIONID[256] = {0};

char GLOBAL_HEADER[256] = {0};
char GLOBAL_HEADER_NO_CACHE_PLAIN_TEXT[512] = { 0 };

char MYSQL_HOST[64] = { 0 };
char MYSQL_PORT_NUMBER[64] = { 0 };
char MYSQL_USERNAME[64] = { 0 };
char MYSQL_PASSWORD[64] = { 0 };
char MYSQL_DBNAME[128] = { 0 };

// HTMLS
const char *BEFORE_TEMPLATE = u8"<div id=\"list_page\" style=\"background-color:transparent !important\"><table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"titleTop2\" style=\"background-color:transparent !important;border:none\"><tbody><tr><td class=\"pageAlign\"><table cellpadding=\"0\" width=\"100%\" class=\"displayTag\" cellspacing=\"1\" border=\"0\" id=\"user\"><thead><tr><th align=\"center\" width=\"20%\" class=\"sortable\">课名</th><th align=\"center\" width=\"10%\" class=\"sortable\">类别</th><th align=\"center\" width=\"10%\" class=\"sortable\">成绩</th><th align=\"center\" width=\"10%\" class=\"sortable\">均分</th><th align=\"center\" width=\"10%\" class=\"sortable\">最高</th><th align=\"center\" width=\"10%\" class=\"sortable\">最低</th><th align=\"center\" width=\"10%\" class=\"sortable\">名次</th><th align=\"center\" width=\"10%\" class=\"sortable\">学分</th><th align=\"center\" width=\"10%\" class=\"sortable\">绩点</th></tr></thead><tbody>";
const char *AFTER_TEMPLATE = u8"</tbody></table></td></tr></tbody></table></div>";
const char *SCORE_TEMPLATE = u8"<tr class=\"even\" onmouseout=\"this.className='even';\" onmouseover=\"this.className='evenfocus';\"><td align=\"center\" style=\"%s\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%.1f</td></tr>";
const char *BEFORE_TEMPLATE_BY_PLAN = u8"<div id=\"list_page\" style=\"background-color:transparent !important\"><table width=\"100%\"border=\"0\"cellpadding=\"0\"cellspacing=\"0\"class=\"titleTop2\"style=\"background-color:transparent !important;border:none\"><tbody><tr><td class=\"pageAlign\"><table cellpadding=\"0\"width=\"100%\"class=\"displayTag\"cellspacing=\"1\"border=\"0\"id=\"user\"><thead><tr><th align=\"center\"width=\"45%\"class=\"sortable\">课名</th><th align=\"center\"width=\"10%\"class=\"sortable\">类别</th><th align=\"center\"width=\"10%\"class=\"sortable\">成绩</th><th align=\"center\"width=\"10%\"class=\"sortable\">学分</th><th align=\"center\"width=\"10%\"class=\"sortable\">绩点</th><th align=\"center\"width=\"15%\"class=\"sortable\">备注</th></tr></thead><tbody>";
const char *SCORE_TEMPLATE_BY_PLAN = u8"<tr class=\"even\"onmouseout=\"this.className='even';\"onmouseover=\"this.className='evenfocus';\"><td align=\"center\"style=\"%s\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%.2f</td><td align=\"center\">%s</td></tr>";
const char *QUICK_SCORE = u8"<div class=\"weui-cell\"><div class=\"weui-cell__bd\"><p>%s</p></div><div class=\"weui-cell__ft\">%s</div></div>";
const char *OAUTH2_LOGIN_HTML = u8"<a id=\"no_ajax\" title=\"QQ登录\" class=\"weui-btn weui-btn_default col-50\" href=\"OAuth2.fcgi\"><i class=\"fa fa-qq\"></i>QQ登录</a>";
const char *QUICKQUERY_HTML = u8"<div class=\"quickquery\"><a class=\"weui-btn weui-btn_warn\" href=\"QuickQuery.fcgi\"><i class=\"fa fa-search\"></i>免密快速查询入口</a></div>";
const char *ASSOC_LINK_HTML = u8"<a id=\"no_ajax\" href=\"index.fcgi?act=requestAssoc\"><i class=\"fa fa-link\"></i>绑定QQ帐号</a>";
const char *RLS_ASSOC_LINK_HTML = u8"<span style=\"color:rgb(0,255,90)\"><i class=\"fa fa-qq\"></i>QQ已绑定</span> · <a id=\"no_ajax\" href=\"javascript:void(0);\" onclick=\"releaseAssoc('%s');\"><i class=\"fa fa-unlink\"></i>解除绑定</a>";
const char *CARD_AD_BANNER_HTML = u8"<div class=\"swiper-slide\"><a id=\"no_ajax\" href=\"%s\" target=\"_blank\"><img data-src=\"%s\" height=\"160\" width=\"100%%\" class=\"swiper-lazy\"></a><div class=\"swiper-lazy-preloader\"></div></div>";
const char *TEST_LIST_HTML = u8"<form id=\"ajax_submit\"data-ajax-submit=\"/query.fcgi?order=tests\"class=\"weui-cells weui-cells_form\"><div class=\"weui-cells__title\">请选择考试类别来查看分数 :)</div><div class=\"signbox\"><div class=\"weui-cell\"><div class=\"weui-cell__hd\"><label for=\"name\"class=\"weui-label\">考试名称</label></div><div class=\"weui-cell__bd\"><input name=\"tests\" class=\"weui-input\" id=\"tests\" type=\"text\" value=\"点击这里选择...\" readonly=\"readonly\"/></div></div></div><div class=\"weui-btn-area\"><button onclick=\"return query_tests();\"type=\"button\"class=\"weui-btn weui-btn_primary\"><i class=\"fa fa-check-square-o\"></i>查询</button></div></form>";
const char *find_user_result_section = "<tr class=\"even\" onmouseout=\"this.className='even';\" onmouseover=\"this.className='evenfocus';\"><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td></tr>";

// 错误页面和配置初始化
std::string ERROR_HTML;
char *HEADER_TEMPLATE_LOCATION = NULL;
char *FOOTER_TEMPLATE_LOCATION = NULL;
char *SERVER_URL = NULL;
char *USER_AGENT = NULL;
int CURL_TIMEOUT = 10;
char *OAUTH2_APPID = NULL;
char *OAUTH2_SECRET = NULL;
bool CURL_USE_PROXY = false;
char *CURL_PROXY_URL = NULL;
char *APP_NAME = NULL;
char *CARD_AD_BANNER_1_IMG = NULL;
char *CARD_AD_BANNER_2_IMG = NULL;
char *CARD_AD_BANNER_1_URL = NULL;
char *CARD_AD_BANNER_2_URL = NULL;
char *ADMIN_USER_NAME = NULL;
char *ADMIN_PASSWORD = NULL;
char *SECONDARY_TITLE = NULL;
char *APP_KEYWORDS = NULL;
char *APP_DESCRIPTION = NULL;
char *FOOTER_TEXT = NULL;
char *ANALYSIS_CODE = NULL;
char *HOMEPAGE_NOTICE = NULL;
char *DISCUSSION_PAGE_CONTENT = NULL;
char *DISCUSSION_PAGE_CODE = NULL;
char *SITE_MAINTENANCE = NULL;
bool ENABLE_QUICK_QUERY = false;

/*
输出错误页面
*/
void Error(const char *p_ErrMsg)
{
	std::cout << GLOBAL_HEADER;
	if (isAjaxRequest)
	{
		std::cout << strformat(error.c_str(), p_ErrMsg).c_str();
	}
	else
	{
		std::cout << strformat(ERROR_HTML.c_str(), p_ErrMsg).c_str();
	}
}

/*
BASE64 编码
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
*/
float cj2jd(float cj)
{
	if (cj <= 59)
		return 0.0;
	return ((((int)cj % 60) / 10.0) + 1.0);
}

/*
实现 std::string 的格式化功能
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