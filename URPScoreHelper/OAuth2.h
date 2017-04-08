#pragma once
#ifndef OAUTH2_H
#define OAUTH2_H

#define OAUTH2_APPID "wxcd594aba810fc332"
#define OAUTH2_SECRET "a976231a15eac68874d8f895ed3d8de6"
#define OAUTH2_AUTHENTICATION "https://open.weixin.qq.com/connect/oauth2/authorize?appid=%s&redirect_uri=%s&response_type=code&scope=snsapi_base&state=%s#wechat_redirect"
#define OAUTH2_ACCESS_TOKEN "https://api.weixin.qq.com/sns/oauth2/access_token?appid=%s&secret=%s&code=%s&grant_type=authorization_code"

void OAuth2_process();
void OAuth2_CallBack();

size_t curl_receive(void *buffer, size_t size, size_t nmemb, char *html);

#endif