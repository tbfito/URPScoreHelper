// URPScoreHelper.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "General.h"
#include "StringHelper.h"
#include "CrawlFactory.h"
#include "URPScoreHelper.h"
#include "resource.h"

using namespace std;

// 入口函数
int main()
{
	g_start_time = GetTickCount();

	// 查找资源
	HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_HTML1), MAKEINTRESOURCE(RT_HTML));
	if (NULL == hRsrc)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>FindResource() 失败！</p>";
		return -1;
	}

	// 获取资源的大小
	DWORD dwSize = SizeofResource(NULL, hRsrc);
	if (0 == dwSize)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>SizeofResource() 失败！</p>";
		return -1;
	}

	// 加载资源
	HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
	if (NULL == hGlobal)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>LoadResource() 失败！</p>";
		return -1;
	}

	// 锁定资源
	ERROR_HTML = (char *)LockResource(hGlobal);
	if (NULL == ERROR_HTML)
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>LockResource() 失败！</p>";
		return -1;
	}

	// 初始化 Socket 库。
	if (!InitSocketLibrary())
	{
		cout << "Status: 500 Internal Server Error\n"
			<< GLOBAL_HEADER
			<< "<p>Socket 初始化失败！</p>";
		return -1;
	}

		CGI_REQUEST_METHOD = getenv("REQUEST_METHOD"); // 请求方法
		CGI_CONTENT_LENGTH = getenv("CONTENT_LENGTH"); // 数据长度
		CGI_SCRIPT_NAME = getenv("SCRIPT_NAME"); // 脚本名称
		CGI_QUERY_STRING = getenv("QUERY_STRING"); // 查询参数
		CGI_PATH_TRANSLATED = getenv("PATH_TRANSLATED"); // 脚本位置
		CGI_HTTP_COOKIE = getenv("HTTP_COOKIE"); // Cookie
		CGI_SERVER_SOFTWARE = getenv("SERVER_SOFTWARE"); // 服务器软件

		// 获取多少用户使用了我们的服务 :)
		g_fQueryCount = fopen("QueryCount.bin", "r+");
		g_QueryCount = 0;
		if (g_fQueryCount != NULL)
		{
			fscanf(g_fQueryCount, "%ld", &g_QueryCount);
		}
		else
		{
			g_fQueryCount = fopen("QueryCount.bin", "w+");
		}
		if (g_fQueryCount == NULL)
		{
			cout << "Status: 500 Internal Server Error\n"
				<< GLOBAL_HEADER
				<< "<p>fopen() 失败！</p>";
			return 0;
		}
		fseek(g_fQueryCount, 0, SEEK_SET);

		if (CGI_REQUEST_METHOD == NULL || CGI_CONTENT_LENGTH == NULL || CGI_SCRIPT_NAME == NULL || CGI_QUERY_STRING == NULL ||
			CGI_PATH_TRANSLATED == NULL || CGI_HTTP_COOKIE == NULL)
		{
			cout << "Status: 500 Internal Server Error\n"
				<< GLOBAL_HEADER
				<< "<p>CGI 接口异常，请检查设置。</p>";
			return 0;
		}

		if (strcmp(CGI_REQUEST_METHOD, "GET") == 0) // 如果是 GET 请求
		{
			if (strcmp(CGI_SCRIPT_NAME, "/") == 0 || strcmp(CGI_SCRIPT_NAME, "/index.cgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=logout") == 0)
				{
					student_logout();
					cout << "Status: 302 Found\n" << "Location: index.cgi\n" << GLOBAL_HEADER;
					return 0;
				}
				parse_index();
				return 0;
			}
			if (strcmp(CGI_SCRIPT_NAME, "/query.cgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=system_registration") == 0)
				{
					system_registration();
					ZeroMemory(JSESSIONID, 256);
					return -1;
				}
				else
				{
					cout << "Status: 405 Method Not Allowed\n";
					Error("<p>该页面不允许 GET 请求。</p>");
					WSACleanup();
					return 0;
				}
			}

			if (strcmp(CGI_SCRIPT_NAME, "/QuickQuery.cgi") == 0)
			{
				parse_QuickQuery_Intro();
				return 0;
			}

			cout << "Status: 404 No Such CGI Page\n";
			Error("<p>找不到该页面。</p>");
			WSACleanup();
			
			ZeroMemory(JSESSIONID, 256);
			return -1;
		}

		if (strcmp(CGI_REQUEST_METHOD, "POST") == 0) // 如果是 POST 请求
		{
			if (strcmp(CGI_SCRIPT_NAME, "/query.cgi") == 0)
			{
				if (strcmp(CGI_QUERY_STRING, "act=QuickQuery") == 0)
				{
					parse_QuickQuery_Result();
					return 0;
				}
				parse_query();
				return 0;
			}
		}

	puts("Status: 500 Internal Server Error");
	Error("<p>发生错误，未经处理的异常。</p>");
	WSACleanup();
	return -1;
}

// 处理 Cookie
int process_cookie(bool *p_need_update_cookie)
{
	int m_iResult = 0;

	if (strcmp(CGI_HTTP_COOKIE, "") != 0) // 如果客户端已经拿到 JSESSIONID
	{
		// 申请内存，并接受服务端返回数据。
		char * m_rep_header = (char *)malloc(1024);
		ZeroMemory(m_rep_header, 1024);
		char m_req_homepage_cookie[2048] = { 0 };
		sprintf(m_req_homepage_cookie, REQUEST_HOME_PAGE_WITH_COOKIE, CGI_HTTP_COOKIE);
		if (!CrawlRequest(m_req_homepage_cookie, m_rep_header, 1024, &m_iResult))
		{
			free(m_rep_header);
			return -1;
		}

		// 看看原 Cookie 是否过期、有效（即服务器是否设置了新 Cookie）。
		char *pStr1 = strstr(m_rep_header, "JSESSIONID=");
		if (pStr1 != NULL)
		{
			char *pStr2 = strstr(pStr1 + 11, ";");
			if (pStr2 == NULL)
			{
				free(m_rep_header);
				WSACleanup();
				cout << "Status: 500 Internal Server Error\n";
				Error("<p>无法获取 Servlet Session ID。</p><p>Cookie 结尾失败。</p>");
				return -1;
			}
			mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // 成功获得新 Session ID。
			*p_need_update_cookie = true;
			free(m_rep_header);
			return 0;
		}
		else // 如果 Cookie 还能用，就获取它。
		{
			right(JSESSIONID, (char *)CGI_HTTP_COOKIE, strlen(CGI_HTTP_COOKIE) - 11);
		}
		free(m_rep_header);
	}
	else
	{
		// 申请内存，并接受服务端返回数据。
		char * m_rep_header = (char *)malloc(1024);
		ZeroMemory(m_rep_header, 1024);
		if (!CrawlRequest(REQUEST_HOME_PAGE, m_rep_header, 1024, &m_iResult))
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
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取 Servlet Session ID。</p><p>Cookie 标头失败。</p>");
			return -1;
		}
		char *pStr2 = strstr(pStr1 + 11, ";");
		if (pStr2 == NULL)
		{
			closesocket(g_so);
			WSACleanup();
			cout << "Status: 500 Internal Server Error\n";
			Error("<p>无法获取 Servlet Session ID。</p><p>Cookie 结尾失败。</p>");
			return -1;
		}
		mid(JSESSIONID, pStr1, pStr2 - pStr1 - 11, 11); // 成功获得 Session ID。

		free(m_rep_header);
		*p_need_update_cookie = true;
	}
}

// 处理主页面请求 (GET / /index.cgi)
int parse_index()
{
	int m_iResult = 0;
	bool m_need_update_cookie = false;

	process_cookie(&m_need_update_cookie);

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
	char *pStr1 = strstr(m_rep_body, "\r\n\r\n");
	if (pStr1 == NULL)
	{
		WSACleanup();
		cout << "Status: 500 Internal Server Error\n";
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
		cout << "Status: 500 Internal Server Error\n";
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
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>无法读取主页模板内容。</p>");
		fclose(m_file_homepage);
		free(m_lpszHomepage);
		WSACleanup();
		return -1;
	}
	fclose(m_file_homepage); // 关闭文件

	 // 填充网页模板
	int m_iBufferSize = m_file_homepage_length + strlen(m_DataURL) + strlen(__DATE__) + strlen(__TIME__)
						+ strlen(__FILE__) + strlen(CGI_SERVER_SOFTWARE); // 获得缓冲区长度

	char *m_lpszCompleteHomepage = (char *)malloc(m_iBufferSize);
	ZeroMemory(m_lpszCompleteHomepage, m_iBufferSize);

	sprintf(m_lpszCompleteHomepage, m_lpszHomepage, g_QueryCount, m_DataURL, 
			__FILE__, __DATE__, __TIME__, CGI_SERVER_SOFTWARE);

	free(m_lpszHomepage);

	// 输出网页
	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";

	cout << GLOBAL_HEADER;
	cout << m_lpszCompleteHomepage;

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
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>发生错误，POST 数据长度异常。</p>");
		WSACleanup();
		return -1;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);	// TORESEARCH
	if (fgets(m_post_data, m_post_length + 1, stdin) == NULL)
	{
		cout << "Status: 500 Internal Server Error\n";
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
		cout << "Status: 500 Internal Server Error\n";
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
		cout << "Status: 500 Internal Server Error\n";
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
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>无法获取验证码信息。</p>");
		return -1;
	}
	char m_captcha[128] = { 0 };
	right(m_captcha, pStr1 + 4, 4);

	if (!student_login(m_xuehao, m_password, m_captcha))
	{
		// 其余资源清理已在学生登录里面做过了。
		free(m_post_data);
		return -1;
	}

	free(m_post_data);

	// 开始查分。
	int m_iResult = 0;
	char QUERY_SCORE[512] = { 0 };
	char *m_rep_body = (char *)malloc(81920);
	sprintf( QUERY_SCORE, REQUEST_QUERY_SCORE, CGI_HTTP_COOKIE );
	if (!CrawlRequest(QUERY_SCORE, m_rep_body, 81920, &m_iResult))
	{
		student_logout();
		free(m_rep_body);
		WSACleanup();
		return -1;
	}
	char *m_result = strstr(m_rep_body, "\r\n\r\n");
	if (m_result == NULL)
	{
		student_logout();
		WSACleanup();
		free(m_post_data);
		free(m_rep_body);
		cout << "Status: 500 Internal Server Error\n";
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
		student_logout();
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
		student_logout();
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
		cout << "Status: 403 Forbidden\n";

		char *m_original_str = "<p><b>亲爱的%s，系统君说你本学期还没有电子注册 0.0</b></p><p>我可以施展法术，\
一键帮你在教务系统注册哦~</p><p>--&gt; 点按下方按钮，直达查分界面 :P &lt;--</p>\
<div class=\"col-100\"><a href=\"query.cgi?act=system_registration\" class=\"button button-big but\
ton-fill button-success\">一键注册</a></div>";

		char m_output_str[1024] = { 0 };
		sprintf(m_output_str, m_original_str, m_Student);
		Error(m_output_str);
		return;
	}

	// 安全登出教务系统。
	student_logout();

	// 定位到第一项成绩
	char *pStr1 = strstr(p_lpszScore,"<tr class=\"odd\" onMouseOut=\"this.className='even';\" onMouseOver=\"this.className='evenfocus';\">");
	if (pStr1 == NULL)
	{
		free(m_lpszQuery);
		Error("<p><b>额，分数页面神隐了 0.0。</b></p><p>这可能是因为新的学期开始了，上次考试成绩存档了哦~ 或者可以试试免密查询功能。</p>");
		return;
	}

	bool m_success = false;
	char m_Output[81920] = { 0 };
	char *pStr2 = NULL;
	char *pStr3 = NULL;

	// 循环获取每一项成绩信息

	double m_Total_xuefen = 0.0;
	double m_Total_pointsxxuefen = 0.0;

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
		if (strstr(m_subchengji, "优秀") != NULL)
		{
			strcpy(m_subchengji,"95");
		}
		if (strstr(m_subchengji, "良好") != NULL)
		{
			strcpy(m_subchengji, "85");
		}
		if (strstr(m_subchengji, "中等") != NULL)
		{
			strcpy(m_subchengji, "75");
		}
		if (strstr(m_subchengji, "及格") != NULL)
		{
			if (atoi(m_subzuidifen) > 60)
			{
				strcpy(m_subchengji, m_subzuidifen);
			}
			else
			{
				strcpy(m_subchengji, "60");
			}
			
		}
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

		// （分数x学分）全都加起来/总学分 = 加权分，排除体育和课程设计
		if (strstr(m_subName, "体育") == NULL && strstr(m_subName, "军事训练") == NULL 
			&& strstr(m_subName, "实践") == NULL)
		{
			float m_xuefen = atof(m_subXuefen);
			float m_chengji = atof(m_subchengji);
			if (m_xuefen != 0) // 排0学分...，并统计总学分
			{
				m_Total_xuefen += m_xuefen;
			}
			if (m_chengji != 0)
			{
				double m_pointsxxuefen = m_xuefen * m_chengji;
				if (m_pointsxxuefen != 0)
				{
					m_Total_pointsxxuefen += m_pointsxxuefen;
				}
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

	// 填充返回页面
	if (m_Total_pointsxxuefen != 0 || m_Total_xuefen != 0)
	{
		char m_jiaquanfen[81920] = { 0 };
		sprintf(m_jiaquanfen, "<div id=\"i_total\"><p>本学期加权综合分（评优、排名依据，供参考）：</p><center>%.1f</center></div>", 
				m_Total_pointsxxuefen / m_Total_xuefen);
		strcat(m_jiaquanfen, m_Output);
		ZeroMemory(m_Output, 81920);
		strcpy(m_Output, m_jiaquanfen);
	}

	char m_query_time[512] = { 0 };
	sprintf(m_query_time, "<center>本次查询耗时 %.2f 秒</center>", (double)((GetTickCount() - g_start_time) / 1000));
	strcat(m_Output, m_query_time);
	char m_lpszCompleteQuery[81920] = { 0 };
	sprintf(m_lpszCompleteQuery, m_lpszQuery, m_Student, m_Student, m_Output);
	
	cout << GLOBAL_HEADER << m_lpszCompleteQuery;

	free(m_lpszQuery);
	fprintf(g_fQueryCount, "%ld", ++g_QueryCount);
	fclose(g_fQueryCount);
}

// 获取学生姓名
void get_student_name(char *p_lpszBuffer)
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
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
	if (strcmp(CGI_HTTP_COOKIE, "") == 0)
	{
		cout << "Status: 302 Found\n" << "Location: index.cgi\n" << GLOBAL_HEADER;
		cout << GLOBAL_HEADER;
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
		student_logout();
		WSACleanup();
		return -1;
	}

	// 查找需要电子注册的学期信息。
	char *pStr1 = strstr(m_rep_header, "selected>");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (1)</p>");
		student_logout();
		WSACleanup();
		return -1;
	}
	pStr1 -= 70;
	char *pStr2 = strstr(pStr1, "<option value=\"");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (2)</p>");
		student_logout();
		WSACleanup();
		return -1;
	}
	pStr1 = pStr2;
	pStr2 = strstr(pStr1 + 16, "\"");
	if (pStr2 == NULL)
	{
		free(m_rep_header);
		Error("<p>数据错误。不好意思，自动注册失败，劳请大佬去教务系统看看吧~ (3)</p>");
		student_logout();
		WSACleanup();
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
		student_logout();
		free(m_rep_header);
		WSACleanup();
		return -1;
	}

	// 检查是否注册成功。
	pStr1 = strstr(m_rep_header, "注册成功");
	if (pStr1 == NULL)
	{
		free(m_rep_header);
		student_logout();
		WSACleanup();
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
		WSACleanup();
		return -1;
	}
	char *m_result = strstr(m_rep_header, "\r\n\r\n");
	if (m_result == NULL)
	{
		student_logout();
		free(m_rep_header);
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>从服务器拉取分数失败。</p>");
		WSACleanup();
		return -1;
	}

	// 优化接受结果，显示查询页面
	parse_friendly_score(m_result);
	free(m_rep_header);
	
	// 完事~
	WSACleanup();
	return 0;
}

// 登录学生
bool student_login(char *p_xuehao, char *p_password, char *p_captcha)
{
	// 发送登陆请求。
	int m_iResult = 0;
	char * m_rep_body = (char *)malloc(40960);
	ZeroMemory(m_rep_body, 40960);
	char POST_LOGIN[1024] = { 0 };
	char *m_origin = "zjh1=&tips=&lx=&evalue=&eflag=&fs=&dzslh=&zjh=%s&mm=%s&v_yzm=%s";
	char m_padding[1024] = { 0 };
	sprintf(m_padding, m_origin, p_xuehao, p_password, p_captcha);
	int m_ContentLength = strlen(m_padding); // TODO: 这里不用加莫名其妙的结束长度
	sprintf(POST_LOGIN, REQUEST_LOGIN, m_ContentLength, CGI_HTTP_COOKIE, p_xuehao, p_password, p_captcha);
	if (!CrawlRequest(POST_LOGIN, m_rep_body, 40960, &m_iResult))
	{
		free(m_rep_body);
		return false;
	}

	// 拉取登录结果。
	char *m_result = strstr(m_rep_body, "\r\n\r\n");
	if (m_result == NULL)
	{
		WSACleanup();
		free(m_rep_body);
		cout << "Status: 500 Internal Server Error\n";
		Error("<p>从服务器拉取登录结果失败。</p>");
		return false;
	}

	// 处理登录结果。
	char *m_login_not_auth = strstr(m_result, "证件号");
	if (m_login_not_auth != NULL)
	{
		WSACleanup();
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>证件号或密码不对啊，大佬。 TAT。</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "密码不正确");
	if (m_login_not_auth != NULL)
	{
		WSACleanup();
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>学号或密码不对啊，大佬。 TAT。</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "验证码错误");
	if (m_login_not_auth != NULL)
	{
		WSACleanup();
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>验证码不对啊，大佬。 TAT。</p>");
		return false;
	}
	m_login_not_auth = strstr(m_result, "数据库");
	if (m_login_not_auth != NULL)
	{
		WSACleanup();
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>教务系统君说数据库繁忙 :P</p><p>对于<b>数据库跑路</b>问题，那就等等先咯~</p>");
		return false;
	}
	char *m_login_success = strstr(m_result, "学分制综合教务");
	if (m_login_success == NULL)
	{
		WSACleanup();
		free(m_rep_body);
		cout << "Status: 403 Forbidden\n";
		Error("<p>天呐。发生了谜一般的问题！教务系统神隐了 0.0</p><p>建议你稍候再试试吧。</p>");
		return false;
	}

	// 至此，学生登录成功，释放资源。
	free(m_rep_body);
	return true;
}

// 登出学生
void student_logout()
{
	if (strcmp(CGI_HTTP_COOKIE, "") == 0) return;

	int m_iResult = 0;
	char m_logout[10240] = { 0 };
	sprintf(m_logout, REQUEST_LOGOUT, CGI_HTTP_COOKIE);
	char *m_outbuffer = (char *)malloc(10240);
	CrawlRequest(m_logout, m_outbuffer, 10240, &m_iResult);
	free(m_outbuffer);
}

// 免密查询入口 (/QuickQuery.cgi)
void parse_QuickQuery_Intro()
{
	bool m_need_update_cookie = false;
	process_cookie(&m_need_update_cookie);

	// 读入页面文件
	FILE *m_file_query = fopen(CGI_PATH_TRANSLATED, "rb");
	if (m_file_query == NULL)
	{
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>错误：找不到免密查询模板。</p>");
		return;
	}
	fseek(m_file_query, 0, SEEK_END); // 移到尾
	int m_file_query_length = ftell(m_file_query); // 获取文件长度
	fseek(m_file_query, 0, SEEK_SET); // 重新移回来
	char *m_lpszQuery = (char *)malloc(m_file_query_length + 1);
	ZeroMemory(m_lpszQuery, m_file_query_length + 1);
	if (fread(m_lpszQuery, m_file_query_length, 1, m_file_query) != 1) // 将硬盘数据拷至内存
	{
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>无法读取免密查询模板内容。</p>");
		fclose(m_file_query);
		free(m_lpszQuery);
		return;
	}
	fclose(m_file_query); // 关闭文件

	int m_ilength = strlen(m_lpszQuery) + 512;
	char *m_lpszCompleteQuery = (char *)malloc(m_ilength);
	ZeroMemory(m_lpszCompleteQuery, m_ilength);
	sprintf(m_lpszCompleteQuery, m_lpszQuery, g_QueryCount);

	if (m_need_update_cookie)
		cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
	cout << GLOBAL_HEADER << m_lpszCompleteQuery;

	free(m_lpszQuery);
	free(m_lpszCompleteQuery);
}

// 免密查询结果 (/query.cgi?act=QuickQuery)
void parse_QuickQuery_Result()
{
	bool m_need_update_cookie = false;
	process_cookie(&m_need_update_cookie);

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

	// 获取 POST 数据。
	int m_post_length = atoi(CGI_CONTENT_LENGTH);
	if (m_post_length <= 0)
	{
		cout << "Status: 500 Internal Server Error\n";
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>发生错误，POST 数据长度异常。</p>");
		WSACleanup();
		return;
	}
	char *m_post_data = (char *)malloc(m_post_length + 2);	// TORESEARCH
	if (fgets(m_post_data, m_post_length + 1, stdin) == NULL)
	{
		cout << "Status: 500 Internal Server Error\n";
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>发生错误，POST 数据拉取失败。</p>");
		WSACleanup();
		return;
	}

	// 获取学号
	char *pStr1 = strstr(m_post_data, "xh=");
	if (pStr1 == NULL)
	{
		WSACleanup();
		free(m_post_data);
		cout << "Status: 500 Internal Server Error\n";
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>无法获取学号信息。</p>");
		return;
	}
	char *pStr2 = strstr(pStr1 + 3, "&");
	char m_xuehao[128] = { 0 };
	right(m_xuehao, pStr1, strlen(pStr1) - 3);
	replace_string(m_xuehao, "%0D%0A", "|");
	char *p = strtok(m_xuehao, "|");
	char *m_xh[10] = { NULL };
	int m_xhgs = 0;
	while (p)
	{
		m_xh[m_xhgs++] = p;
		p = strtok(NULL, "|");
	}
	pStr1 = NULL;
	pStr2 = NULL;
	if (m_xhgs > 5 || m_xhgs <= 0)
	{
		WSACleanup();
		free(m_post_data);
		cout << "Status: 500 Internal Server Error\n";
		if (m_need_update_cookie)
			cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
		Error("<p>输入的学号个数存在问题，请确认！</p>");
		return;
	}

	char m_list[102400] = { 0 };
	char m_xxmz[128] = { 0 };
	free(m_post_data);

	for (int xh_index = 0; xh_index < m_xhgs; xh_index++)
		{
			if (strlen(m_xh[xh_index]) != 9)
			{
				WSACleanup();
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>输入的学号中有长度存在问题，请确认！</p>");
				return;
			}

			char m_query_param[1024] = { 0 };
			sprintf(m_query_param, "LS_XH=%s", m_xh[xh_index]);
			strcat(m_query_param, "&resultPage=http%3A%2F%2Fjw0.yzu.edu.cn%3A80%2FreportFiles%2Fcj%2Fcj_zwcjd.jsp%3F");
			char m_query_request[2048] = { 0 };
			sprintf(m_query_request, REQUEST_SET_REPORT_PARAMS, CGI_HTTP_COOKIE, strlen(m_query_param));
			strcat(m_query_request, m_query_param);

			int m_ilength = 0;
			char *m_lpvBuffer = (char *)malloc(4096);
			ZeroMemory(m_lpvBuffer, 4096);
			if (!CrawlRequest(m_query_request, m_lpvBuffer, 4096, &m_ilength))
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>投递免密查询请求失败。</p><p>请确认教务系统是可用的。</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}

			pStr1 = strstr(m_lpvBuffer, "&reportParamsId=");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>获取报表ID错误。(1)</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}
			pStr2 = strstr(pStr1 + 16, "\n");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>获取报表ID错误。(2)</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}
			char m_paramsID[512] = { 0 };
			mid(m_paramsID, pStr1 + 16, pStr2 - pStr1 - 16, 0);
			pStr1 = NULL;
			pStr2 = NULL;
			free(m_lpvBuffer);

			char m_query_report[512] = { 0 };
			sprintf(m_query_report, REQUEST_REPORT_FILES, m_paramsID, CGI_HTTP_COOKIE);
			m_lpvBuffer = (char *)malloc(40960);
			ZeroMemory(m_lpvBuffer, 40960);
			if (!CrawlRequest(m_query_report, m_lpvBuffer, 40960, &m_ilength))
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>通过ID免密查询失败。</p><p>发生了天知道的错误。</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}

			pStr1 = strstr(m_lpvBuffer, "Exception: ");
			if (pStr1 != NULL)
			{
				pStr2 = strstr(pStr1, "at");
				if (pStr2 != NULL)
				{
					cout << "Status: 500 Internal Server Error\n";
					if (m_need_update_cookie)
						cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
					char m_Exception[512] = { 0 };
					mid(m_Exception, pStr1 + 11, pStr2 - pStr1 - 11, 0);
					char m_ExceptionMsg[512] = "<p><b>教务系统抛出了如下错误...</b></p><p>";
					strcat(m_ExceptionMsg, m_Exception);
					strcat(m_ExceptionMsg, "</p>");
					Error(m_ExceptionMsg);
					WSACleanup();
					free(m_lpvBuffer);
					return;
				}
			}
			pStr1 = NULL;
			pStr2 = NULL;

			pStr1 = strstr(m_lpvBuffer, "com.runqian.report.view.text.TextFileServlet");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>免密查询返回参数错误。(1)</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}
			pStr2 = strstr(pStr1 + 46, "\";");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>免密查询返回参数错误。(2)</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}
			char m_txt_req_path[512] = { 0 };
			mid(m_txt_req_path, pStr1 + 45, pStr2 - pStr1 - 45, 0);
			free(m_lpvBuffer);

			char m_query_score[512] = { 0 };
			sprintf(m_query_score, REQUEST_TXT_SCORES, m_txt_req_path, m_paramsID, CGI_HTTP_COOKIE);
			m_lpvBuffer = (char *)malloc(40960);
			ZeroMemory(m_lpvBuffer, 40960);
			if (!CrawlRequest(m_query_score, m_lpvBuffer, 40960, &m_ilength))
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>接受分数信息失败！</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}

			pStr1 = strstr(m_lpvBuffer, "姓名\t");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>学生姓名获取失败！(1)</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}
			pStr2 = strstr(pStr1 + 4, "\t\t");
			if (pStr2 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>学生姓名获取失败！(2)</p>");
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}

			if ((pStr2 - pStr1) <= 4)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error, 
					"<p><b>呃，获取失败了。请确认所输信息是正确的。</b></p><p>发生错误的学号: %s</p>", 
					m_xh[xh_index]);
				Error(m_friendly_error);
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}

			mid(m_xxmz, pStr1 + 4, pStr2 - pStr1 - 5, 1);
			if (strlen(m_xxmz) < 2)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					"<p><b>获取信息失败，请确认输入正确。</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}
			char m_xxmz_html[128] = { 0 };
			sprintf(m_xxmz_html, "<div class=\"content-block-title\">%s</div>", m_xxmz);

			pStr1 = NULL;
			pStr2 = NULL;
			pStr1 = strstr(m_lpvBuffer, "考试时间\t\t\n");
			if (pStr1 == NULL)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					"<p><b>接受到的报表存在问题。</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}

			if (strlen(m_lpvBuffer) <= 800)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				char m_friendly_error[512] = { 0 };
				sprintf(m_friendly_error,
					"<p><b>收到的报表大小存在问题。</b></p><p>发生错误的学号: %s</p>",
					m_xh[xh_index]);
				Error(m_friendly_error);
				WSACleanup();
				free(m_lpvBuffer);
				return;
			}

			pStr1 += 11;
			pStr2 = strstr(pStr1, "\t\t\t\t");
			bool m_success = true;
			strcat(m_list, m_xxmz_html);
			strcat(m_list, "<div class=\"list-block\"><ul>");
			test_info m_test_info[256];
			int m_index = 0;

			while (pStr2 != NULL)
			{
				char m_kcmz[128] = { 0 };
				mid(m_kcmz, pStr1, pStr2 - pStr1, 0);
				//cout << m_kcmz;

				pStr1 = pStr2 + 4;
				pStr2 = strstr(pStr1, "\t");
				if (pStr2 == NULL)
				{
					m_success = false;
					break;
				}

				char m_xf[64] = { 0 };
				mid(m_xf, pStr1, pStr2 - pStr1, 0);
				pStr1 = pStr2 + 1; // 学分
				pStr2 = strstr(pStr1, "\t");
				if (pStr2 == NULL)
				{
					m_success = false;
					break;
				}

				char m_cj[64] = { 0 };
				mid(m_cj, pStr1, pStr2 - pStr1, 0);
				if (atoi(m_cj) < 60)
				{
					char m_red[64] = "<b style=\"color:#f6383a\">";
					strcat(m_red, m_cj);
					strcat(m_red, "</b>");
					strcpy(m_cj, m_red);
				}

				//cout << " - " << m_cj;
				pStr2 = strstr(pStr2 + 1, "\t");
				if (pStr2 == NULL)
				{
					m_success = false;
					break;
				}

				char m_lb[64] = { 0 };
				mid(m_lb, pStr1, pStr2 - pStr1, 0);
				if (strstr(m_lb, "重修") != NULL)
				{
					char m_kcmz_cx[256] = { 0 };
					strcat(m_kcmz_cx, "[重修] ");
					strcat(m_kcmz_cx, m_kcmz);
					strcpy(m_kcmz, m_kcmz_cx);
				}

				//cout << " - " << m_lb;
				pStr1 = pStr2 + 6;
				pStr2 = strstr(pStr1, "\t\t");
				if (pStr2 == NULL)
				{
					m_success = false;
					break;
				}

				char m_date[128] = { 0 };
				mid(m_date, pStr1, pStr2 - pStr1, 0);
				char m_date_4[64] = { 0 };
				left(m_date_4, m_date, 6);


				//cout << " - " << m_date << endl;
				pStr1 = pStr2 + 2;
				pStr2 = strstr(pStr1, "\t\t\t\t");

				if (atoi(m_xf) == 0 && atoi(m_cj) == 0)
				{
					break;
				}

				strcpy(m_test_info[m_index].cj, m_cj);
				strcpy(m_test_info[m_index].kcmz, m_kcmz);
				m_test_info[m_index].date = atoi(m_date_4);
				m_index++;
			}

			free(m_lpvBuffer);

			if (!m_success)
			{
				cout << "Status: 500 Internal Server Error\n";
				if (m_need_update_cookie)
					cout << "Set-Cookie: JSESSIONID=" << JSESSIONID << "; path=/\n";
				Error("<p>抱歉，免密查询失败，请稍后再试。</p>");
				WSACleanup();
				return;
			}

			int m_max_date = 0;
			int m_secondary_max = 0;

			for (int i = 0; i < m_index; i++)
			{
				if (m_test_info[i].date > m_max_date)
				{
					m_max_date = m_test_info[i].date;
				}
			}

			for (int i = 0; i < m_index; i++)
			{
				if (m_test_info[i].date != m_max_date && m_test_info[i].date > m_secondary_max)
				{
					m_secondary_max = m_test_info[i].date;
				}
			}

			int m_interval = m_max_date - m_secondary_max;

			for (int i = 0; i < m_index; i++)
			{
				if (m_interval > 90)
				{
					if (m_test_info[i].date == m_max_date)
					{
						char m_temp[1024] = { 0 };
						sprintf(m_temp, QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
						strcat(m_list, m_temp);
					}
				}
				else if (m_test_info[i].date == m_max_date || m_test_info[i].date == m_secondary_max)
				{
					char m_temp[1024] = { 0 };
					sprintf(m_temp, QUICK_SCORE, m_test_info[i].kcmz, m_test_info[i].cj);
					strcat(m_list, m_temp);
				}
			}
			strcat(m_list, "</ul></div>");
		}
		cout << GLOBAL_HEADER;
		char m_query_time[512] = { 0 };
		sprintf(m_query_time, "<center>本次查询耗时 %.2f 秒</center>", (double)((GetTickCount() - g_start_time) / 1000));
		strcat(m_list, m_query_time);
		char m_outer[204800] = { 0 };
		if (m_xhgs > 1)
		{
			sprintf(m_outer, m_lpszQuery, "多人查询", "多人查询", m_list);
		}
		else
		{
			sprintf(m_outer, m_lpszQuery, m_xxmz, m_xxmz, m_list);
		}

		cout << m_outer;
		g_QueryCount = g_QueryCount + m_xhgs;
		fprintf(g_fQueryCount, "%ld", g_QueryCount);
		fclose(g_fQueryCount);
}