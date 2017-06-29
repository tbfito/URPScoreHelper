#include "stdafx.h"
#include "General.h"
#include "CrawlFactory.h"

void InitSocketLibrary()
{
	// 初始化 Windows Socket 库。
	WSADATA WSAData;
	int m_dwRet = WSAStartup(MAKEWORD(2, 2), &WSAData);
}

bool CrawlRequest(const char *p_rq, char *p_lpvBuffer, int p_iLength, int *p_iTotalRead, bool no_error_page)
{
	// 创建 IPv4 TCP 套接字对象。
	g_so = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (g_so == INVALID_SOCKET)
	{
		char m_ErrMsg[1024] = { 0 };
		std::cout << "Status: 500 Internal Server Error\r\n";
		sprintf(m_ErrMsg, "<p>无法创建 Socket。</p>\r\n<p>错误代码： %d</p>", WSAGetLastError());
		Error(m_ErrMsg);
		return false;
	}

	// 连接学院教务系统 Apache 服务器
	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_addr.S_un.S_addr = inet_addr(SERVER);
	m_sockaddr.sin_port = htons(atoi(SERVER_PORT));

	int error = -1;
	int len = sizeof(int);
	timeval tm;
	fd_set set;
	unsigned long ul = 1;
	ioctlsocket(g_so, FIONBIO, &ul); //设置为非阻塞模式
	bool ret = false;

	int m_iResult = connect(g_so, (sockaddr *)&m_sockaddr, sizeof(m_sockaddr));
	if (m_iResult == SOCKET_ERROR)
	{
		tm.tv_sec = 1;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(g_so, &set);
		if (select(g_so + 1, NULL, &set, NULL, &tm) > 0)
		{
			getsockopt(g_so, SOL_SOCKET, SO_ERROR, (char *)&error, /*(socklen_t *)*/&len);
			if (error == 0)
				ret = true;
			else
				ret = false;
		}
		else
			ret = false;
	}
	else
	{
		ret = true;
	}
	ul = 0;
	ioctlsocket(g_so, FIONBIO, &ul);
	if(!ret)
	{
		int errid = WSAGetLastError();
		closesocket(g_so);
		if (!no_error_page)
		{
			std::cout << "Status: 500 Internal Server Error\r\n";
			char buff[10] = { 0 };
			_itoa(errid, buff, 10);
			std::string err_msg("<p><b>连接学校服务器失败！</b></p><p>OS代码: (");
			err_msg.append(buff);
			err_msg.append(") ");
			char *wsamsg = (char *)malloc(MAX_PATH);
			FormatMessageA(4096, NULL, errid, NULL, wsamsg, MAX_PATH, NULL);
			err_msg.append(wsamsg);
			if (errid == 0)
				err_msg.append("/ 连接超时");
			err_msg.append("</p><p>可能学校服务器挂了，我能怎么办，我也很绝望？</p>");
			free(wsamsg);
			Error((char *)err_msg.c_str());
		}
		return false;
	}
	// 发送首页请求，相当于访问 http://SERVER_IP/ 。
	m_iResult = send(g_so, p_rq, strlen(p_rq), 0);
	if (m_iResult != strlen(p_rq))
	{
		closesocket(g_so);
		if (!no_error_page)
		{
			std::cout << "Status: 500 Internal Server Error\r\n";
			char m_ErrMsg[1024] = { 0 };
			sprintf(m_ErrMsg, "<p>无法向教务系统投递请求。</p>\r\n<p>错误代码： %d</p>", WSAGetLastError());
			Error(m_ErrMsg);
		}
		return false;
	}

	// 接受数据，写入缓冲区。
	m_iResult = recv(g_so, p_lpvBuffer, p_iLength, 0); /* 此处会适当阻塞，接受服务器返回 */
	while (m_iResult > 0)
	{
		*p_iTotalRead += m_iResult;
		m_iResult = recv(g_so, p_lpvBuffer + *p_iTotalRead, p_iLength - *p_iTotalRead, 0); /* 此处会适当阻塞，接受服务器返回 */
	}
	if (m_iResult != SOCKET_ERROR)
	{
		*p_iTotalRead += m_iResult;
	}

	closesocket(g_so);
	return true;
}

/*
bool CurlCrawl(const char *url, bool isPOST = false, const char *postdata = "", const char *cookie = "", struct curl_slist *list = NULL)
{
	CURL* curl = curl_easy_init();
	if (curl == NULL)
	{
		return false;
	}

	char* html = (char *)malloc(1024);
	memset(html, 0, 1024);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	if (isPOST)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
	}
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, html);
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_receive);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "iEdon-URPScoreHelper");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	CURLcode ret = curl_easy_perform(curl);

	if (ret != CURLE_OK)
	{
		return false;
	}
}
int shift = 0;
size_t curl_receive(void *buffer, size_t size, size_t nmemb, char *html)
{
	int resize = size * nmemb;
	html = (char *)realloc(html, resize + shift + 1);
	memcpy(html + shift, (char *)buffer, resize);
	shift += resize;
	return resize;
}
*/