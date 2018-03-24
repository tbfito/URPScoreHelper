#include "headers.h"
#include "General.h"
#include "StringHelper.h"
#include "ctype.h"
#include "Encrypt.h"

char JSESSIONID[1024] = {0};

char GLOBAL_HEADER[256] = {0};
char GLOBAL_HEADER_TYPE_PLAIN_TEXT[512] = { 0 };

char APP_SUB_DIRECTORY[MAX_PATH] = { 0 };
char MYSQL_HOST[1024] = { 0 };
char MYSQL_PORT_NUMBER[1024] = { 0 };
char MYSQL_USERNAME[1024] = { 0 };
char MYSQL_PASSWORD[1024] = { 0 };
char MYSQL_DBNAME[1024] = { 0 };

// HTMLS
const char *BEFORE_TEMPLATE = u8"<div id=\"list_page\" style=\"background-color:transparent !important\"><table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"titleTop2\" style=\"background-color:transparent !important;border:none\"><tbody><tr><td class=\"pageAlign\"><table cellpadding=\"0\" width=\"100%\" class=\"displayTag\" cellspacing=\"1\" border=\"0\" id=\"user\"><thead><tr><th align=\"center\" width=\"20%\" class=\"sortable\">课名</th><th align=\"center\" width=\"10%\" class=\"sortable\">类别</th><th align=\"center\" width=\"10%\" class=\"sortable\">成绩</th><th align=\"center\" width=\"10%\" class=\"sortable\">均分</th><th align=\"center\" width=\"10%\" class=\"sortable\">最高</th><th align=\"center\" width=\"10%\" class=\"sortable\">最低</th><th align=\"center\" width=\"10%\" class=\"sortable\">名次</th><th align=\"center\" width=\"10%\" class=\"sortable\">学分</th><th align=\"center\" width=\"10%\" class=\"sortable\">绩点</th></tr></thead><tbody>";
const char *AFTER_TEMPLATE = u8"</tbody></table></td></tr></tbody></table></div>";
const char *SCORE_TEMPLATE = u8"<tr class=\"even\" onmouseout=\"this.className='even';\" onmouseover=\"this.className='evenfocus';\"><td align=\"center\" style=\"%s\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%.1f</td></tr>";
const char *BEFORE_TEMPLATE_BY_PLAN = u8"<div id=\"list_page\" style=\"background-color:transparent !important\"><table width=\"100%\"border=\"0\"cellpadding=\"0\"cellspacing=\"0\"class=\"titleTop2\"style=\"background-color:transparent !important;border:none\"><tbody><tr><td class=\"pageAlign\"><table cellpadding=\"0\"width=\"100%\"class=\"displayTag\"cellspacing=\"1\"border=\"0\"id=\"user\"><thead><tr><th align=\"center\"width=\"45%\"class=\"sortable\">课名</th><th align=\"center\"width=\"10%\"class=\"sortable\">类别</th><th align=\"center\"width=\"10%\"class=\"sortable\">成绩</th><th align=\"center\"width=\"10%\"class=\"sortable\">学分</th><th align=\"center\"width=\"10%\"class=\"sortable\">绩点</th><th align=\"center\"width=\"15%\"class=\"sortable\">备注</th></tr></thead><tbody>";
const char *SCORE_TEMPLATE_BY_PLAN = u8"<tr class=\"even\"onmouseout=\"this.className='even';\"onmouseover=\"this.className='evenfocus';\"><td align=\"center\"style=\"%s\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%.2f</td><td align=\"center\">%s</td></tr>";
const char *QUICK_SCORE = u8"<div class=\"weui-cell\"><div class=\"weui-cell__bd\"><p>%s</p></div><div class=\"weui-cell__ft\">%s</div></div>";
const char *OAUTH2_LOGIN_HTML = u8"<a id=\"no_ajax\" title=\"微信登录\" class=\"weui-btn weui-btn_default col-50\" href=\"OAuth2.fcgi\"><i class=\"fa fa-weixin\"></i>微信登录</a>";
const char *QUICKQUERY_HTML = u8"<a class=\"weui-btn quickquery\" href=\"QuickQuery.fcgi\"><i class=\"fa fa-search\"></i>学号快速查询入口</a>";
const char *ASSOC_LINK_HTML = u8"<a id=\"no_ajax\" href=\"index.fcgi?act=linking\"><i class=\"fa fa-link\"></i>绑定微信</a>";
const char *RLS_ASSOC_LINK_HTML = u8"<span style=\"color:rgb(0,255,90)\"><i class=\"fa fa-weixin\"></i>微信已绑定</span>&nbsp;&nbsp;<a id=\"no_ajax\" href=\"javascript:void(0);\" onclick=\"unlink('%s');\"><i class=\"fa fa-unlink\"></i>解除绑定</a>";
const char *CARD_AD_BANNER_HTML = u8"<div class=\"swiper-slide\"><a id=\"no_ajax\" href=\"%s\" target=\"_blank\"><img data-src=\"%s\" height=\"160\" width=\"100%%\" class=\"swiper-lazy\"></a><div class=\"swiper-lazy-preloader\"></div></div>";
const char *TEST_LIST_HTML = u8"<form id=\"ajax_submit\"data-ajax-submit=\"query.fcgi?order=tests\"class=\"weui-cells weui-cells_form\"><div class=\"weui-cells__title\">请选择一场考试：</div><div class=\"signbox\"><div class=\"weui-cell\"><div class=\"weui-cell__hd\"><label for=\"name\"class=\"weui-label\">考试名称</label></div><div class=\"weui-cell__bd\"><input name=\"tests\" class=\"weui-input\" id=\"tests\" type=\"text\" value=\"点击这里选择...\" readonly=\"readonly\"/></div></div></div><div class=\"weui-btn-area\"><button onclick=\"return query_tests();\"type=\"button\"class=\"weui-btn weui-btn_primary\"><i class=\"fa fa-check-square-o\"></i>查询</button></div></form>";
const char *FIND_USER_RESULT_SECTION = "<tr class=\"even\" onmouseout=\"this.className='even';\" onmouseover=\"this.className='evenfocus';\"><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td></tr>";
const char *LOGGED_USER_HTML = "<div class=\"logged-user\"><img src=\"%s\" class=\"avatar\" height=\"64\" width=\"64\"/><p>%s</p></div>";

// 错误页面和配置初始化
std::string ERROR_HTML;
char *HEADER_TEMPLATE_LOCATION = NULL;
char *FOOTER_TEMPLATE_LOCATION = NULL;
char *SERVER_URL = NULL;
char *USER_AGENT = NULL;
int CURL_CONN_TIMEOUT;
bool ENABLE_OAUTH2 = false;
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
	if (CGI_HTTP_X_IS_AJAX_REQUEST)
	{
		std::cout << strformat(error.c_str(), p_ErrMsg).c_str();
	}
	else
	{
		std::cout << strformat(ERROR_HTML.c_str(), p_ErrMsg).c_str();
	}
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

/* 获取应用的URL */
std::string getAppURL()
{
	std::string text;
	bool isHTTPS = false;

	if (CGI_HTTPS != NULL)
	{
		std::string https(CGI_HTTPS);
		if (!https.empty())
		{
			std::transform(https.begin(), https.end(), https.begin(), tolower);
			if (https == "on" || https == "1") {
				isHTTPS = true;
			}
		}
	}
	if (CGI_HTTP_FORWARDED != NULL)
	{
		std::string proto = _COOKIE(CGI_HTTP_FORWARDED, "proto"); // 解析 Forwarded 字段中 proto 值的方法与取 Cookie 的办法相同，可以直接套用
		if (!proto.empty())
		{
			std::transform(proto.begin(), proto.end(), proto.begin(), tolower);
			if (proto == "https") {
				isHTTPS = true;
			}
		}
	}
	if(CGI_HTTP_X_FORWARDED_PROTO != NULL)
	{
		std::string proto(CGI_HTTP_X_FORWARDED_PROTO);
		std::transform(proto.begin(), proto.end(), proto.begin(), tolower);
		if (proto == "https") {
			isHTTPS = true;
		}
	}

	text.append(isHTTPS ? "https://" : "http://");
	text.append(CGI_HTTP_HOST);
	if (strlen(APP_SUB_DIRECTORY) >= 2) { // 如果子目录不为空且输入有效，还需带上子目录
		text.append(APP_SUB_DIRECTORY);
	}

	return text;
}

/* 输出 token Cookie 头 */
void output_token_header(const char *m_xuehao, const char *m_password)
{
	std::cout << "Set-Cookie: token=" << generate_token(m_xuehao, m_password).c_str() << "; max-age=604800; path=" << APP_SUB_DIRECTORY << "/\r\n";
}

/* 生成 token */
std::string generate_token(const char *m_xuehao, const char *m_password)
{
	char m_xuehaoe[4096] = { 0 };
	char m_passworde[4096] = { 0 };
	EnCodeStr(m_xuehao, m_xuehaoe);
	EnCodeStr(m_password, m_passworde);
	std::string token(m_xuehaoe);
	token += "-";
	token += m_passworde;
	char token_e[10240] = { 0 };
	strncpy(token_e, token.c_str(), sizeof(token_e) - 1);
	EnCodeStr(token_e, token_e);
	std::string ret(token_e);
	return ret;
}

/* 解开 token */
void decode_token(char *token, std::string & token_xh, std::string & token_mm)
{
	token_xh.erase();
	token_mm.erase();
	DeCodeStr(token);
	char *xh = (char *)malloc(MAX_PATH);
	char *mm = (char *)malloc(MAX_PATH);
	if (sscanf(token, "%[^-]-%s", xh, mm) == 2)
	{
		DeCodeStr(xh);
		DeCodeStr(mm);
		token_xh = xh;
		token_mm = mm;
	}
	free(xh);
	free(mm);
}