#include "stdafx.h"
#include "General.h"
#include "CCurlTask.h"

CCurlTask::CCurlTask()
{
	curl = curl_easy_init();
}

CCurlTask::~CCurlTask()
{
	if (curl != NULL)
	{
		curl_easy_cleanup(curl);
		curl = NULL;
	}
}

char * CCurlTask::GetResult()
{
	return (char *)result.c_str();
}

size_t CCurlTask::GetLength()
{
	return result.length();
}

std::string & CCurlTask::GetStrResult()
{
	return result;
}

bool CCurlTask::SetReferer(std::string & referer)
{
	return (curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str()) == CURLE_OK);
}

bool CCurlTask::Exec(bool headonly, std::string url, std::string cookie, bool isPOST, std::string postdata)
{
	if (curl == NULL)
	{
		return false;
	}
	url.insert(0, SERVER_URL);
	if (headonly)
	{
		curl_easy_setopt(curl, CURLOPT_HEADER, true); //返回头信息
		curl_easy_setopt(curl, CURLOPT_NOBODY, true); //不返回内容
	}
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	if (isPOST)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	if (!cookie.empty())
	{
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->curl_receive);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
	if (strlen(CURL_PROXY_URL) != 0)
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, CURL_PROXY_URL);
	}
	CURLcode ret = curl_easy_perform(curl);
	if (ret != CURLE_OK)
	{
		return false;
	}
	return true;
}

size_t CCurlTask::curl_receive(void *buffer, size_t size, size_t nmemb, void *curlclass)
{
	int block_size = size * nmemb;
	CCurlTask * curl = (CCurlTask *)curlclass;
	curl->GetStrResult().append((char*)buffer, block_size);
	return block_size;
}