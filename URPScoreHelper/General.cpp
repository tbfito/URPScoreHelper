#include "stdafx.h"
#include "General.h"

using namespace std;

SOCKET g_so = INVALID_SOCKET;
char JSESSIONID[256] = {0};

const char* GLOBAL_HEADER = "X-Powered-By: iEdon-URPScoreHelper\n\
Content-Type: text/html; charset=gb2312\n\
Cache-Control: no-cache\n\
Pragma: no-cache\n\
Expires: -1\n\n";

// 首页 HTTP 请求
const char* REQUEST_HOME_PAGE = "\
GET / HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Accept: */*\n\
Connection: close\n\n";

// 首页 HTTP 请求（带 COOKIE ）
const char* REQUEST_HOME_PAGE_WITH_COOKIE = "\
GET / HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Accept: */*\n\
Cookie: %s\n\
Connection: close\n\n";

// 验证码 HTTP 请求
const char* REQUEST_CAPTCHA = "\
GET /validateCodeAction.do?random=0.%d HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Accept: */*\n\
Cookie: JSESSIONID=%s\n\
Connection: close\n\n";

// 请求登陆
const char * REQUEST_LOGIN = "\
POST /loginAction.do HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Content-Length: %d\n\
Cache-Control: max-age=0\n\
Content-Type: application/x-www-form-urlencoded\n\
Accept: */*\n\
Cookie: %s\n\
Connection: close\n\n\
zjh1=&tips=&lx=&evalue=&eflag=&fs=&dzslh=&zjh=%s&mm=%s&v_yzm=%s";

// 请求查分
const char * REQUEST_QUERY_SCORE = "\
GET /bxqcjcxAction.do HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Accept: */*\n\
Cookie: %s\n\
Connection: close\n\n";

// 安全登出教务系统
const char * REQUEST_LOGOUT = "\
GET /logout.do HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Accept: */*\n\
Cookie: %s\n\
Connection: close\n\n";

// 获取电子注册页面
const char * REQUEST_GET_REGISTER_INTERFACE = "\
GET /dzzcAction.do HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Accept: */*\n\
Cookie: %s\n\
Connection: close\n\n";

// 提交电子注册请求
const char * REQUEST_POST_REGISTER_INTERFACE = "\
POST /dzzcAction.do?zc=zc&zxjxjhh=%s HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Accept: */*\n\
Cookie: %s\n\
Content-Length: %d\n\
Connection: close\n\n%s";

const char * REQUEST_SET_REPORT_PARAMS =  "\
POST /setReportParams HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Content-Length: %d\n\
Cache-Control: max-age=0\n\
Content-Type: application/x-www-form-urlencoded\n\
Connection: close\n\n";

const char * REQUEST_REPORT_FILES = "\
GET /reportFiles/cj/cj_zwcjd.jsp?&reportParamsId=%s HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Referer: /reportFiles/cj/cj_zwcjd.jsp\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * REQUEST_TXT_SCORES = "\
GET /servlet/com.runqian.report.view.text.TextFileServlet?%s HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Referer: /reportFiles/cj/cj_zwcjd.jsp?&reportParamsId=%s\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * REQUEST_PHOTO = "\
GET /xjInfoAction.do?oper=img HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * REQUEST_TOP = "\
GET /menu/top.jsp HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * GET_GRADE_BY_QBINFO = "\
GET /gradeLnAllAction.do?type=ln&oper=qbinfo HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * GET_GRADE_BY_PLAN = "\
GET /gradeLnAllAction.do?type=ln&oper=fainfo HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * GET_GRADE_BY_FAILED = "\
GET /gradeLnAllAction.do?type=ln&oper=bjg HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * GET_SMALL_TEST_SCORE = "\
GET /cjSearchAction.do?oper=getKscjList HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * GET_TEACH_EVAL_LIST = "\
GET /jxpgXsAction.do?oper=listWj&pageSize=200 HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

const char * POST_TEACH_EVAL = "\
POST /jxpgXsAction.do?oper=wjpg HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Content-Length: %d\n\
Cache-Control: max-age=0\n\
Content-Type: application/x-www-form-urlencoded\n\
Accept: */*\n\
Cookie: %s\n\
Connection: close\n\n%s";

const char * POST_PRE_TEACH_EVAL = "\
POST /jxpgXsAction.do HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Content-Length: %d\n\
Cache-Control: max-age=0\n\
Content-Type: application/x-www-form-urlencoded\n\
Accept: */*\n\
Cookie: %s\n\
Connection: close\n\n%s";

const char * REQ_CHANGE_PASSWORD = "\
GET /modifyPassWordAction.do?pwd=%s HTTP/1.0\n\
User-Agent: iEdon-URPScoreHelper\n\
Cookie: %s\n\
Accept: */*\n\
Connection: close\n\n";

// 分数显示块
const char *BEFORE_TEMPLATE = "<div id=\"list_page\" style=\"background-color:transparent !important\"><table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"titleTop2\" style=\"background-color:transparent !important;border:none\"><tbody><tr><td class=\"pageAlign\"><table cellpadding=\"0\" width=\"100%\" class=\"displayTag\" cellspacing=\"1\" border=\"0\" id=\"user\"><thead><tr><th align=\"center\" width=\"30%\" class=\"sortable\">课程名称</th><th align=\"center\" width=\"10%\" class=\"sortable\">成绩</th><th align=\"center\" width=\"10%\" class=\"sortable\">均分</th><th align=\"center\" width=\"10%\" class=\"sortable\">最高</th><th align=\"center\" width=\"10%\" class=\"sortable\">最低</th><th align=\"center\" width=\"10%\" class=\"sortable\">名次</th><th align=\"center\" width=\"10%\" class=\"sortable\">学分</th><th align=\"center\" width=\"10%\" class=\"sortable\">绩点</th></tr></thead><tbody>";
const char *AFTER_TEMPLATE = "</tbody></table></td></tr></tbody></table></div>";
const char *SCORE_TEMPLATE = "<tr class=\"even\" onmouseout=\"this.className='even';\" onmouseover=\"this.className='evenfocus';\"><td align=\"center\" style=\"%s\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%s</td><td align=\"center\">%.1f</td></tr>";
const char *QUICK_SCORE = "<li class=\"item-content\"><div class=\"item-media\"><i class=\"icon icon-f7\"></i></div><div class=\"item-inner\"><div class=\"item-title\">%s</div><div class=\"item-after\">%s</div></div></li>";

// 错误页面和配置初始化
char *ERROR_HTML = NULL;
char *SERVER = NULL;
char *SERVER_PORT = NULL;
char *OAUTH2_APPID = NULL;
char *OAUTH2_SECRET = NULL;

void Error( char *p_ErrMsg )
{
	int m_PageSize = strlen(ERROR_HTML) + strlen(p_ErrMsg) + 1;
	char *m_ErrPage = (char *)malloc(m_PageSize);
	ZeroMemory(m_ErrPage, m_PageSize);
	sprintf(m_ErrPage, ERROR_HTML, p_ErrMsg);
	cout << GLOBAL_HEADER << m_ErrPage;
	free(m_ErrPage);
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

float cj2jd(float cj)
{
	if (cj <= 59)
		return 0;
	return ((((int)cj % 60) / 10) + 1.0);
}