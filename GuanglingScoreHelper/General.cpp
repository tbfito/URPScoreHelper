#include "stdafx.h"
#include "General.h"

SOCKET g_so = INVALID_SOCKET;
char JSESSIONID[256] = {0};

const char* GLOBAL_HEADER = "Cache-Control: no-cache\n\
Pragma: no-cache\n\
Expires: Thu, 16 Oct 1997 06:00:00 GMT\n\
X-Powered-By: GuanglingScoreHelper\n\
Content-Type: text/html; charset=gb2312\n\n";

// 首页 HTTP 请求
const char* REQUEST_HOME_PAGE = "\
GET / HTTP/1.0\r\n\
Host: 58.220.248.249\r\n\
User-Agent: iEdon-GuanglingScoreHelper\r\n\
Accept: */*\r\n\
Connection: close\r\n\r\n";

// 验证码 HTTP 请求
const char* REQUEST_CAPTCHA = "\
GET /validateCodeAction.do?random=0.%d HTTP/1.0\r\n\
Host: 58.220.248.249\r\n\
User-Agent: iEdon-GuanglingScoreHelper\r\n\
Accept: */*\r\n\
Referer: http://58.220.248.249/\r\n\
Cookie: JSESSIONID=%s\r\n\
Connection: close\r\n\r\n";

// 请求登陆
const char * REQUEST_LOGIN = "\
POST /loginAction.do HTTP/1.0\r\n\
Host: 58.220.248.249\r\n\
User-Agent: iEdon-GuanglingScoreHelper\r\n\
Content-Length: %d\r\n\
Cache-Control: max-age=0\r\n\
Origin: http://58.220.248.249\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Accept: */*\r\n\
Referer: http://58.220.248.249/\r\n\
Cookie: %s\r\n\
Connection: close\r\n\r\n\
zjh1=&tips=&lx=&evalue=&eflag=&fs=&dzslh=&zjh=%s&mm=%s&v_yzm=%s\
";

// 请求查分
const char * REQUEST_QUERY_SCORE = "\
GET /bxqcjcxAction.do HTTP/1.0\r\n\
Host: 58.220.248.249\r\n\
User-Agent: iEdon-GuanglingScoreHelper\r\n\
Accept: */*\r\n\
Cookie: %s\r\n\
Connection: close\r\n\r\n\
";

// 安全登出教务系统
const char * REQUEST_LOGOUT = "\
GET /logout.do HTTP/1.0\r\n\
Host: 58.220.248.249\r\n\
User-Agent: iEdon-GuanglingScoreHelper\r\n\
Accept: */*\r\n\
Cookie: %s\r\n\
Connection: close\r\n\r\n\
";

// 获取电子注册页面
const char * REQUEST_GET_REGISTER_INTERFACE = "\
GET /dzzcAction.do HTTP/1.0\r\n\
Host: 58.220.248.249\r\n\
User-Agent: iEdon-GuanglingScoreHelper\r\n\
Accept: */*\r\n\
Cookie: %s\r\n\
Connection: close\r\n\r\n\
";

// 提交电子注册请求
const char * REQUEST_POST_REGISTER_INTERFACE = "\
POST /dzzcAction.do?zc=zc&zxjxjhh=%s HTTP/1.0\r\n\
Host: 58.220.248.249\r\n\
User-Agent: iEdon-GuanglingScoreHelper\r\n\
Accept: */*\r\n\
Cookie: %s\r\n\
Content-Length: %d\r\n\
Connection: close\r\n\r\n\
%s\
";

// 分数显示块
const char *SCORE_TEMPLATE = "<div class=\"content-block-title\">%s</div><div class=\"list-block\"><ul><li class=\"item-content\"><div class=\"item-media\"><i class=\"icon icon-f7\"></i></div><div class=\"item-inner\"><div class=\"item-title\">成绩</div><div class=\"item-after\">%s</div></div></li><li class=\"item-content\"><div class=\"item-media\"><i class=\"icon icon-f7\"></i></div><div class=\"item-inner\"><div class=\"item-title\">平均分</div><div class=\"item-after\">%s</div></div></li><li class=\"item-content\"><div class=\"item-media\"><i class=\"icon icon-f7\"></i></div><div class=\"item-inner\"><div class=\"item-title\">最高分</div><div class=\"item-after\">%s</div></div></li><li class=\"item-content\"><div class=\"item-media\"><i class=\"icon icon-f7\"></i></div><div class=\"item-inner\"><div class=\"item-title\">最低分</div><div class=\"item-after\">%s</div></div></li><li class=\"item-content\"><div class=\"item-media\"><i class=\"icon icon-f7\"></i></div><div class=\"item-inner\"><div class=\"item-title\">名次</div><div class=\"item-after\">%s</div></div></li><li class=\"item-content\"><div class=\"item-media\"><i class=\"icon icon-f7\"></i></div><div class=\"item-inner\"><div class=\"item-title\">所得学分</div><div class=\"item-after\">%s</div></div></li></ul></div>";

// 错误页面
const char *ERROR_PAGE = "<!DOCTYPE html><html><head><meta charset=\"gb2312\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><title>出错啦TAT</title><meta name=\"viewport\" content=\"initial-scale=1,maximum-scale=1\"><link rel=\"shortcut icon\" href=\"/favicon.ico\"><meta name=\"apple-mobile-web-app-capable\" content=\"yes\"><meta name=\"apple-mobile-web-app-status-bar-style\" content=\"black\"><link rel=\"stylesheet\" href=\"css/sm.min.css\"><link rel=\"stylesheet\" href=\"css/sm-extend.min.css\"></head><body><div class=\"page-group\"><div class=\"page\"><header class=\"bar bar-nav\"><button class=\"button button-link button-nav pull-left\"><span class=\"icon icon-left\"></span><a href=\"index.cgi\">返回</a></button><h1 class=\"title\">出错啦 TAT</h1></header><nav class=\"bar bar-tab\"><a class=\"tab-item external active\" href=\"#\"><span class=\"icon icon-star\"></span><span class=\"tab-label\">成绩查询</span></a></nav><div class=\"content\"><div class=\"card demo-card-header-pic\"><div class=\"card-content\"><div class=\"card-content-inner\">%s</div></div><div valign=\"bottom\" class=\"card-header color-white no-border no-padding\"><img class=\"card-cover\" src=\"error.jpg\" alt=\"\"></div></div></div></div><script type=\"text/javascript\" src=\"js/zepto.min.js\" charset=\"utf-8\"></script><script type=\"text/javascript\" src=\"js/sm.min.js\" charset=\"utf-8\"></script><script type=\"text/javascript\" src=\"js/sm-extend.min.js\" charset=\"utf-8\"></script></div></body></html>";

void Error( char *p_ErrMsg )
{
	char m_ErrPage[2048] = { 0 };
	sprintf(m_ErrPage, ERROR_PAGE, p_ErrMsg);
	char m_ErrMsg[2048] = {0};
	strcat( m_ErrMsg, GLOBAL_HEADER );
	strcat( m_ErrMsg, m_ErrPage );
	puts( m_ErrMsg );
}

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
