#pragma once
class CCurlTask {
public:
	CCurlTask();
	~CCurlTask();
	bool Exec(bool headonly, std::string url, std::string cookie = "", bool isPOST = false, std::string postdata = "");
	bool CCurlTask::SetReferer(std::string & referer);
	char *GetResult();
	size_t GetLength();
	std::string & GetStrResult();
	
private:
	static size_t curl_receive(void *buffer, size_t size, size_t nmemb, void *curlclass);
	CURL* curl;
	std::string result;
};