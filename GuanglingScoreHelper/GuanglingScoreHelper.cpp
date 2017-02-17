// GuanglingScoreHelper.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "General.h"
#include "StringHelper.h"
#include "CrawlFactory.h"
#include "GuanglingScoreHelper.h"
#include "resource.h"

// 入口函数
int main()
{
	CGI_REQUEST_METHOD = getenv("REQUEST_METHOD"); // 请求方法
	CGI_CONTENT_LENGTH = getenv("CONTENT_LENGTH"); // 数据长度
	CGI_SCRIPT_NAME = getenv("SCRIPT_NAME"); // 脚本名称
	CGI_QUERY_STRING = getenv("QUERY_STRING"); // 查询参数
	CGI_PATH_TRANSLATED = getenv("PATH_TRANSLATED"); // 脚本位置
	CGI_HTTP_COOKIE = getenv("HTTP_COOKIE"); // Cookie

	// 获取多少用户使用了我们的服务 :)
	g_fQueryCount = fopen("QueryCount.log","r+");
	g_QueryCount = 0;
	if (g_fQueryCount != NULL) 
	{
		fscanf(g_fQueryCount, "%ld", &g_QueryCount);
	}
	else
	{
		g_fQueryCount = fopen("QueryCount.log", "w+");
	}
	if (g_fQueryCount == NULL)
	{
		puts("Status: 500 Internal Server Error\n");
		puts("<p>fopen() 失败！</p>");
		return -1;
	}
	fseek(g_fQueryCount, 0, SEEK_SET);

	// 查找资源
	HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_HTML1), MAKEINTRESOURCE(RT_HTML));
	if (NULL == hRsrc)
	{
		puts("Status: 500 Internal Server Error\n");
		puts("<p>FindResoure() 失败！</p>");
		return -1;
	}

	// 获取资源的大小
	DWORD dwSize = SizeofResource(NULL, hRsrc);
	if (0 == dwSize)
	{
		puts("Status: 500 Internal Server Error\n");
		puts("<p>SizeofResource() 失败！</p>");
		return -1;
	}

	// 加载资源
	HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
	if (NULL == hGlobal)
	{
		puts("Status: 500 Internal Server Error\n");
		puts("<p>LoadResoure() 失败！</p>");
		return -1;
	}

	// 锁定资源
	ERROR_HTML = (char *)LockResource(hGlobal);
	if (NULL == ERROR_HTML)
	{
		puts("Status: 500 Internal Server Error\n");
		puts("<p>LockResoure() 失败！</p>");
		return -1;
	}

	if (CGI_REQUEST_METHOD == NULL || CGI_CONTENT_LENGTH == NULL || CGI_SCRIPT_NAME == NULL || CGI_QUERY_STRING == NULL ||
		CGI_PATH_TRANSLATED == NULL || CGI_HTTP_COOKIE == NULL)
	{
		puts("Status: 500 Internal Server Error\n");
		puts("<p>CGI 接口异常，请检查设置！</p>");
		return -1;
	}

	// 初始化 Socket 库。
	if (!InitSocketLibrary())
	{
		return -1;
	}

	if ( strcmp( CGI_REQUEST_METHOD, "GET" ) == 0 ) // 如果是 GET 请求
	{
		if (strcmp(CGI_SCRIPT_NAME, "/") == 0 || strcmp(CGI_SCRIPT_NAME, "/index.cgi") == 0)
		{
			return parse_index();
		}
		if (strcmp(CGI_SCRIPT_NAME, "/query.cgi") == 0)
		{
			if (strcmp(CGI_QUERY_STRING, "act=system_registration") == 0)
			{
				return system_registration();
			}
			else 
			{
				puts("Status: 405 Method Not Allowed");
				Error("<p>该页面不允许 GET 请求。</p>");
				WSACleanup();
				return 1;
			}
		}
		puts("Status: 404 No Such CGI Page");
		Error("<p>找不到该页面。</p>");
		WSACleanup();
		return 1;
	}

	if ( strcmp( CGI_REQUEST_METHOD, "POST" ) == 0 ) // 如果是 POST 请求
	{
		if ( strcmp( CGI_SCRIPT_NAME, "/query.cgi" ) == 0 )
		{
			return parse_query();
		}
	}

	puts("Status: 500 Internal Server Error");
	Error("<p>发生错误，未经处理的异常。</p>");
	WSACleanup();
    return 1;
}

// 处理主页面请求 (GET / /index.cgi)
int parse_index()
{
	int m_iResult = 0;

	// 申请内存，并接受服务端返回数据。
	char * m_rep_header = (char *)malloc(512);
	ZeroMemory(m_rep_header, 512);
	if (!CrawlRequest(REQUEST_HOME_PAGE, m_rep_header, 512, &m_iResult))
	{
		free(m_rep_header);
		return -1;
	}

	// 获取 Session ID。
	char *pStr1 = strstr(m_rep_header, "JSESSIONID=");
	if (pStr1 == NULL)
	{
		closesocket(g_so);
		WSACleanup();
		puts("Status: 500 Internal Server Error");
		Error("<p>无法获取 Servlet Session ID。</p><p>Cookie 标头失败。</p>");
		return -1;
	}
	char *pStr2 = strstr(pStr1 + 11, ";");
	if (pStr2 == NULL)
	{
		closesocket(g_so);
		WSACleanup();
		puts("Status: 500 Internal Server Error");
		Error("<p>无法获取 Servlet Session ID。</p><p>Cookie 结尾失败。</p>");
		return -1;
	}
	mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // 成功获得 Session ID。
	//printf(JSESSIONID);
	//printf("\n");
	free(m_rep_header);

	// 置随机数种子，并取得一个随机数，用于获取验证码。
	srand((int)time(0));
	int m_rand = rand();
	char Captcha[256] = { 0 };
	sprintf(Captcha, REQUEST_CAPTCHA, m_rand, JSESSIONID);

	// 发送验证码请求，获取验证码数据。
	char * m_rep_body = (char *)malloc(8192);
	ZeroMemory(m_rep_body, 8192);
	if (!CrawlRequest(Captcha, m_rep_body, 8192, &m_iResult))
	{
		free(m_rep_body);
		return -1;
	}

	// 从返回数据流中获取验证码图片。
	pStr1 = strstr(m_rep_body, "\r\n\r\n");
	if (pStr1 == NULL)
	{
		WSACleanup();
		puts("Status: 500 Internal Server Error");
		Error("<p>无法分析验证码响应协议。</p>");
		return -1;
	}
	pStr1 += 4; // 指针后移四位，指向 jpg 开始位置。
	int m_CaptchaLength = m_iResult - (pStr1 - m_rep_body); // 验证码图片的大小

	// 将验证码转化为 base64 编码后的 DataURL，浏览器直接显示，供用户查看。
	char m_base64[8192] = { 0 };
	base64_encode((const unsigned char *)pStr1, m_base64, m_CaptchaLength);
	char m_DataURL[10240] = "data:image/jpg;base64,";
	strcat(m_DataURL, m_base64);
	free(m_rep_body);

	// 读入主页面文件
	FILE *m_file_homepage = fopen(CGI_PATH_TRANSLATED, "rb");
	if (m_file_homepage == NULL)
	{
		puts("Status: 500 Internal Server Error");
		Error("<p>错误：找不到主页面模板。</p>");
		return -1;
	}
	fseek(m_file_homepage, 0, SEEK_END); // 移到尾
	int m_file_homepage_length = ftell(m_file_homepage); // 获取文件长度
	fseek(m_file_homepage, 0, SEEK_SET); // 重新移回来
	char *m_lpszHomepage = (char *)malloc(m_file_homepage_length + 1);
	ZeroMemory(m_lpszHomepage, m_file_homepage_length + 1);
	if (fread(m_lpszHomepage, m_file_homepage_length, 1, m_file_homepage) != 1) // 将硬盘数据拷至内存
	{
		puts("Status: 500 Internal Server Error");
		Error("<p>无法读取主页模板内容。</p>");
		fclose(m_file_homepage);
		free(m_lpszHomepage);
		WSACleanup();
		return -1;
	}
	fclose(m_file_homepage); // 关闭文件

	 // 填充网页模板
	int m_iBufferSize = m_file_homepage_length + strlen(m_DataURL); // 获得缓冲区长度
	char *m_lpszCompleteHomepage = (char *)malloc(m_iBufferSize);
	ZeroMemory(m_lpszCompleteHomepage, m_iBufferSize);
	sprintf(m_lpszCompleteHomepage, m_lpszHomepage, g_QueryCount, m_DataURL);
	free(m_lpszHomepage);

	// 输出网页
	printf("Set-Cookie: JSESSIONID=%s; path=/\n", JSESSIONID);
	puts(GLOBAL_HEADER);
	puts(m_lpszCompleteHomepage);

	// 释放资源
	free(m_lpszCompleteHomepage);
	WSACleanup();
	return 0;
}

// 处理查询页面请求 (POST /query.cgi)
int parse_query()
{
	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		puts("Status: 500 Internal Server Error");
		Error("<p>发生错误，POST 数据长度异常。</p>");
		WSACleanup();
		return -1;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);	// TODO: 为什么这里是+2而不能+1？貌似CONTENTLENGTH之后还有1个结束符？
	if (fgets(m_post_data, m_post_length + 1 , stdin) == NULL) // TODO: CONTENTLENGTH之后还有1个结束符？
	{
		puts("Status: 500 Internal Server Error");
		Error("<p>发生错误，POST 数据拉取失败。</p>");
		WSACleanup();
		return -1;
	}

	// 获取学号
	char *pStr1 = strstr(m_post_data, "xh=");
	if (pStr1 == NULL)
	{
		WSACleanup();
		free(m_post_data);
		puts("Status: 500 Internal Server Error");
		Error("<p>无法获取学号信息。</p>");
		return -1;
	}
	char *pStr2 = strstr(pStr1 + 3, "&");
	char m_xuehao[128] = { 0 };
	mid(m_xuehao, pStr1, pStr2 - pStr1 - 3, 3);
	pStr1 = NULL;
	pStr2 = NULL;

	// 获取密码
	pStr1 = strstr(m_post_data, "mm=");
	if (pStr1 == NULL)
	{
		WSACleanup();
		free(m_post_data);
		puts("Status: 500 Internal Server Error");
		Error("<p>无法获取密码信息。</p>");
		return -1;
	}
	pStr2 = strstr(pStr1 + 3, "&");
	char m_password[128] = { 0 };
	mid(m_password, pStr1, pStr2 - pStr1 - 3, 3);
	pStr1 = NULL;
	pStr2 = NULL;

	// 获取验证码
	pStr1 = strstr(m_post_data, "yzm=");
	if (pStr1 == NULL)
	{
		WSACleanup();
		free(m_post_data);
		puts("Status: 500 Internal Server Error");
		Error("<p>无法获取验证码信息。</p>");
		return -1;
	}
	char m_captcha[128] = { 0 };
	right(m_captcha, pStr1 + 4, 4);

	// 发送登陆请求。
	int m_iResult = 0;
	char * m_rep_body = (char *)malloc(40960);
	ZeroMemory(m_rep_body, 40960);
	char POST_LOGIN[1024] = { 0 };
	char *m_origin = "zjh1=&tips=&lx=&evalue=&eflag=&fs=&dzslh=&zjh=%s&mm=%s&v_yzm=%s";
	char m_padding[1024] = { 0 };
	sprintf(m_padding, m_origin, m_xuehao, m_password, m_captcha);
	int m_ContentLength = strlen(m_padding); // TODO: 这里不用加莫名其妙的结束长度
	sprintf( POST_LOGIN, REQUEST_LOGIN, m_ContentLength, CGI_HTTP_COOKIE, m_xuehao, m_password, m_captcha );
	if (!CrawlRequest( POST_LOGIN, m_rep_body, 40960, &m_iResult ))
	{
		free(m_rep_body);
		return -1;
	}

	// 拉取登录结果。
	char *m_result = strstr(m_rep_body, "\r\n\r\n");
	if (m_result == NULL)
	{
		WSACleanup();
		free(m_post_data);
		free(m_rep_body);
		puts("Status: 500 Internal Server Error");
		Error("<p>从服务器拉取登录结果失败。</p>");
		return -1;
	}

	// 处理登录结果。
	char *m_login_not_auth = strstr(m_result, "证件号");
	if (m_login_not_auth != NULL)
	{
		WSACleanup();
		free(m_post_data);
		free(m_rep_body);
		puts("Status: 403 Forbidden");
		Error("<p>学号或密码不对啊，大佬。 TAT。</p><p>默认密码是就是学号哦，如果你修改过密码，你懂的~</p>");
		return -1;
	}
	m_login_not_auth = strstr(m_result, "验证码");
	if (m_login_not_auth != NULL)
	{
		WSACleanup();
		free(m_post_data);
		free(m_rep_body);
		puts("Status: 403 Forbidden");
		Error("<p>验证码不对啊，大佬。 TAT。</p>");
		return -1;
	}
	m_login_not_auth = strstr(m_result, "数据库");
	if (m_login_not_auth != NULL)
	{
		WSACleanup();
		free(m_post_data);
		free(m_rep_body);
		puts("Status: 403 Forbidden");
		Error("<p>教务系统君说数据库繁忙 :P</p><p>对于<b>数据库跑路</b>问题，那就等等先咯~</p>");
		return -1;
	}
	char *m_login_success = strstr(m_result, "学分制综合教务");
	if (m_login_success == NULL)
	{
		WSACleanup();
		free(m_post_data);
		free(m_rep_body);
		puts("Status: 403 Forbidden");
		Error("<p>天呐。发生了谜一般的问题！教务系统神隐了 0.0</p>");
		return -1;
	}
	
	// 至此，学生登录成功，释放资源。
	free(m_post_data);
	free(m_rep_body);

	// 开始查分。
	char QUERY_SCORE[512] = { 0 };
	m_rep_body = (char *)malloc(81920);
	sprintf( QUERY_SCORE, REQUEST_QUERY_SCORE, CGI_HTTP_COOKIE );
	if (!CrawlRequest(QUERY_SCORE, m_rep_body, 81920, &m_iResult))
	{
		free(m_rep_body);
		return -1;
	}
	m_result = strstr(m_rep_body, "\r\n\r\n");
	if (m_result == NULL)
	{
		WSACleanup();
		free(m_post_data);
		free(m_rep_body);
		puts("Status: 500 Internal Server Error");
		Error("<p>从服务器拉取分数失败。</p>");
		return -1;
	}

	// 优化接受结果，显示查询页面
	parse_friendly_score(m_result);
	free(m_rep_body);

	// 处理完毕。
	WSACleanup();
	return 0;
}

// 输出分数页面
void parse_friendly_score(char *p_lpszScore)
{
	char m_Student[64] = { 0 };
	get_student_name(m_Student);

	// 读入分数页面文件
	FILE *m_file_query = fopen(CGI_PATH_TRANSLATED, "rb");
	if (m_file_query == NULL)
	{
		Error("<p>错误：找不到分数页面模板。</p>");
		return;
	}
	fseek(m_file_query, 0, SEEK_END); // 移到尾
	int m_file_query_length = ftell(m_file_query); // 获取文件长度
	fseek(m_file_query, 0, SEEK_SET); // 重新移回来
	char *m_lpszQuery = (char *)malloc(m_file_query_length + 1);
	ZeroMemory(m_lpszQuery, m_file_query_length + 1);
	if (fread(m_lpszQuery, m_file_query_length, 1, m_file_query) != 1) // 将硬盘数据拷至内存
	{
		Error("<p>无法读取分数页模板内容。</p>");
		fclose(m_file_query);
		free(m_lpszQuery);
		return;
	}
	fclose(m_file_query); // 关闭文件

	char *m_query_not_reg = strstr(p_lpszScore, "没有注册");
	if (m_query_not_reg != NULL)
	{
		free(m_lpszQuery);
		puts("Status: 403 Forbidden");
		char *m_original_str = "<p><b>亲爱的%s，教务系统君说你本学期还没有电子注册 0.0</b></p><p>我可以施展法术，一键帮你在教务系统注册哦~</p>\
				<div class=\"col-100\"><a href=\"query.cgi?act=system_registration\" class=\"button button-big but\
				ton-fill button-success\">一键注册</a></div>";
		char m_output_str[1024] = { 0 };
		sprintf(m_output_str, m_original_str, m_Student);
		Error(m_output_str);
		return;
	}

	// 定位到第一项成绩
	char *pStr1 = strstr(p_lpszScore,"<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
	if (pStr1 == NULL)
	{
		free(m_lpszQuery);
		puts(GLOBAL_HEADER);
		puts(p_lpszScore);
		Error("<p>额，分数页面神隐了 0.0。稍后再试试吧~</p>");
		return;
	}

	bool m_success = false;
	char m_Output[81920] = { 0 };
	char *pStr2 = NULL;
	char *pStr3 = NULL;

	// 循环获取每一项成绩信息

	double m_Total_xuefen = 0.000000;
	double m_Total_pointsxxuefen = 0.000000;

	while (pStr1 != NULL) 
	{
		pStr2 = pStr1;
		for (int i = 0; i < 3; i++)
		{
			if (pStr2 == NULL) break;
			pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		}
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subName[512] = { 0 };
		mid(m_subName, pStr2, pStr3 - pStr2 - 19, 19);

		pStr2 = pStr3;
		pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subXuefen[128] = { 0 };
		mid(m_subXuefen, pStr2, pStr3 - pStr2 - 19, 19);
		if (atof(m_subXuefen) == 0) sprintf(m_subXuefen, "暂无数据");

		pStr2 = pStr3;
		pStr2 = strstr(pStr2 + 19, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuigaofen[128] = { 0 };
		mid(m_subzuigaofen, pStr2, pStr3 - pStr2 - 19, 19);
		if (atof(m_subzuigaofen) == 0) sprintf(m_subzuigaofen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subzuidifen[128] = { 0 };
		mid(m_subzuidifen, pStr2, pStr3 - pStr2 - 19, 19);
		if (atof(m_subzuidifen) == 0) sprintf(m_subzuidifen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subjunfen[128] = { 0 };
		mid(m_subjunfen, pStr2, pStr3 - pStr2 - 19, 19);
		if (atof(m_subjunfen) == 0) sprintf(m_subjunfen, "暂无数据");

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_subchengji[256] = { 0 };
		mid(m_subchengji, pStr2, pStr3 - pStr2 - 19, 19);
		//if (atoi(m_subchengji) == 0) strcpy(m_subchengji, "暂无数据");
		if (atof(m_subchengji) < 60) 
		{

			char m_completecj[256] = "<b style=\"color:#f6383a\">";
			strcat(m_completecj, m_subchengji);
			strcat(m_completecj, "</b>");
			ZeroMemory(m_subchengji, 256);
			strcpy(m_subchengji, m_completecj);
		}

		pStr2 = strstr(pStr3, "<td align=\"center\">");
		if (pStr2 == NULL) break;
		pStr3 = strstr(pStr2, "</td>");
		if (pStr3 == NULL) break;
		char m_submingci[128] = { 0 };
		mid(m_submingci, pStr2, pStr3 - pStr2 - 19, 19);
		if (atof(m_submingci) == 0) sprintf(m_submingci, "暂无数据");

		char m_StrTmp[10240] = { 0 };
		sprintf(m_StrTmp, SCORE_TEMPLATE, m_subName, m_subchengji, m_subjunfen, m_subzuigaofen, m_subzuidifen,
				m_submingci, m_subXuefen);
		strcat(m_Output, m_StrTmp);

		// （分数x学分）全都加起来/总学分 = 加权分
		float m_xuefen = atof(m_subXuefen);
		if (m_xuefen != 0)
		{
			m_Total_xuefen += m_xuefen;
		}
		float m_chengji = atof(m_subchengji);
		if (m_chengji != 0)
		{
			double m_pointsxxuefen = m_xuefen * m_chengji;
			if (m_pointsxxuefen != 0)
			{
				m_Total_pointsxxuefen += m_pointsxxuefen;
			}
		}

		m_success = true; // 查到一个算一个
		pStr1 = strstr(pStr3, "<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");

	}

	// 假如发生了错误
	if (!m_success) 
	{
		free(m_lpszQuery);
		Error("<p>内个，发生意外错误啦。</p>");
		return;
	}

	puts(GLOBAL_HEADER);

	// 填充返回页面
	if (m_Total_pointsxxuefen != 0 || m_Total_xuefen != 0)
	{
		char m_jiaquanfen[81920] = { 0 };
		sprintf(m_jiaquanfen, "<div id=\"i_total\"><p>本学期学业综合分（评优依据，仅供参考）：</p><center>%.1f</center></div>", 
				m_Total_pointsxxuefen / m_Total_xuefen);
		strcat(m_jiaquanfen, m_Output);
		ZeroMemory(m_Output, 81920);
		strcpy(m_Output, m_jiaquanfen);
	}
	char m_lpszCompleteQuery[81920] = { 0 };
	sprintf(m_lpszCompleteQuery, m_lpszQuery, m_Student, m_Student, m_Output);
	
	free(m_lpszQuery);

	fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
	fclose(g_fQueryCount);

	puts(m_lpszCompleteQuery);

	// 安全登出教务系统。
	int m_iResult = 0;
	char m_logout[1024] = { 0 };
	sprintf(m_logout, REQUEST_LOGOUT, CGI_HTTP_COOKIE);
	char *m_outbuffer = (char *)malloc(1024);
	if (!CrawlRequest(m_logout, m_outbuffer, 40960, &m_iResult))
	{
		free(m_outbuffer);
		return;
	}
	free(m_outbuffer);

}

// 获取学生姓名
void get_student_name(char *p_lpszBuffer)
{
	if (strlen(CGI_HTTP_COOKIE) == 0)
	{
		return;
	}

	int m_iResult = 0;
	char * m_rep_header = (char *)malloc(10240);
	ZeroMemory(m_rep_header, 10240);
	char GET_TOP[1024] = { 0 };
	sprintf(GET_TOP, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE);
	if (!CrawlRequest(GET_TOP, m_rep_header, 10240, &m_iResult))
	{
		free(m_rep_header);
		return;
	}

	// 拉取学生姓名
	char *pStr1 = strstr(m_rep_header, "<td class=\"fieldName\">姓名:&nbsp;</td>");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		return;
	}
	pStr1 = strstr(pStr1 + 39, "<td>");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		return;
	}
	char *pStr2 = strstr(pStr1 + 5,"</td>");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		return;
	}
	mid(p_lpszBuffer, pStr1, pStr2 - pStr1 - 4, 4);
	free(m_rep_header);
}

// 教务系统电子注册 (GET /query.cgi?act=system_registration)
int system_registration()
{
	if (strlen(CGI_HTTP_COOKIE) == 0)
	{
		puts("Status: 302 Found\nLocation: index.cgi\n");
		puts(GLOBAL_HEADER);
		return -1;
	}

	int m_iResult = 0;

	// 发送请求，获取电子注册信息。
	char * m_rep_header = (char *)malloc(10240);
	ZeroMemory(m_rep_header, 10240);
	char m_req[1024] = { 0 };
	sprintf(m_req, REQUEST_GET_REGISTER_INTERFACE, CGI_HTTP_COOKIE);
	if (!CrawlRequest(m_req, m_rep_header, 10240, &m_iResult))
	{
		free(m_rep_header);
		return -1;
	}

	// 查找需要电子注册的学期信息。
	char *pStr1 = strstr(m_rep_header, "selected>");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (1)</p>");
		return -1;
	}
	pStr1 -= 70;
	char *pStr2 = strstr(pStr1, "<option value=\"");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (2)</p>");
		return -1;
	}
	pStr1 = pStr2;
	pStr2 = strstr(pStr1 + 16, "\"");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (3)</p>");
		return -1;
	}

	char m_regval[4096] = { 0 };
	mid(m_regval, pStr1, pStr2 - pStr1 - 15, 15);
	free(m_rep_header);

	// 填充电子注册信息
	char m_post_reg_info[4096] = "zxjxjhh=";
	strcat(m_post_reg_info, m_regval);
	int m_post_reg_info_length = strlen(m_post_reg_info);

	// 填充注册请求
	char m_post_req[8192] = { 0 };
	sprintf(m_post_req, REQUEST_POST_REGISTER_INTERFACE, m_regval, CGI_HTTP_COOKIE, m_post_reg_info_length, 
			m_post_reg_info);

	// 开始电子注册
	m_rep_header = (char *)malloc(10240);
	ZeroMemory(m_rep_header, 10240);
	if (!CrawlRequest(m_post_req, m_rep_header, 10240, &m_iResult))
	{
		free(m_rep_header);
		return -1;
	}

	// 检查是否注册成功。
	pStr1 = strstr(m_rep_header, "注册成功");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		Error("<p>不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (4)</p>");
		return -1;
	}

	// 注册成功了。
	free(m_rep_header);

	// 为刚电子注册的同学查询成绩
	char QUERY_SCORE[512] = { 0 };
	m_rep_header = (char *)malloc(81920);
	sprintf(QUERY_SCORE, REQUEST_QUERY_SCORE, CGI_HTTP_COOKIE);
	if (!CrawlRequest(QUERY_SCORE, m_rep_header, 81920, &m_iResult))
	{
		free(m_rep_header);
		return -1;
	}
	char *m_result = strstr(m_rep_header, "\r\n\r\n");
	if (m_result == NULL)
	{
		WSACleanup();
		free(m_rep_header);
		puts("Status: 500 Internal Server Error");
		Error("<p>从服务器拉取分数失败。</p>");
		return -1;
	}

	// 优化接受结果，显示查询页面
	parse_friendly_score(m_result);
	free(m_rep_header);

	// 完事~。
	WSACleanup();
	return 0;
}
