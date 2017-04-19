#include "stdafx.h"
#include "General.h"
#include "CrawlFactory.h"

bool InitSocketLibrary()
{
	// 初始化 Windows Socket 库。
	WSADATA WSAData;
	int m_dwRet = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (m_dwRet != 0)
	{
		puts("Status: 500 Internal Server Error");
		Error("<p>无法初始化 Windows Socket。</p>");
		return false;
	}
	return true;
}

bool CrawlRequest(const char *p_rq, char *p_lpvBuffer, int p_iLength, int *p_iTotalRead)
{
	// 创建 IPv4 TCP 套接字对象。
	g_so = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (WSAGetLastError() == WSANOTINITIALISED) // 没有初始化或因 WSACleanup() 已清理环境
	{
		InitSocketLibrary(); // 再次初始化
		g_so = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	if (g_so == INVALID_SOCKET)
	{
		WSACleanup();
		char m_ErrMsg[1024] = { 0 };
		puts("Status: 500 Internal Server Error");
		sprintf(m_ErrMsg, "<p>无法创建 Socket。</p>\r\n<p>错误代码： %d</p>", WSAGetLastError());
		Error(m_ErrMsg);
		return false;
	}

	// 连接学院教务系统 Apache 服务器
	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_addr.S_un.S_addr = inet_addr(SERVER);
	m_sockaddr.sin_port = htons(80);

	int m_iResult = connect(g_so, (sockaddr *)&m_sockaddr, sizeof(m_sockaddr));
	if (m_iResult == SOCKET_ERROR)
	{
		closesocket(g_so);
		WSACleanup();
		puts("Status: 500 Internal Server Error");
		Error("<p>连接教务系统服务器失败！</p>");
		return false;
	}

	// 发送首页请求，相当于访问 http://SERVER_IP/ 。
	m_iResult = send(g_so, p_rq, strlen(p_rq), 0);
	if (m_iResult != strlen(p_rq))
	{
		closesocket(g_so);
		WSACleanup();
		puts("Status: 500 Internal Server Error");
		char m_ErrMsg[1024] = { 0 };
		sprintf(m_ErrMsg, "<p>无法向教务系统投递请求。</p>\r\n<p>错误代码： %d</p>", WSAGetLastError());
		Error(m_ErrMsg);
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
	WSACleanup();
	return true;
}
