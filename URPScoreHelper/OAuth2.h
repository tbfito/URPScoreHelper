#pragma once
#ifndef OAUTH2_H
#define OAUTH2_H

#define OAUTH2_AUTHENTICATION "https://graph.qq.com/oauth2.0/authorize?response_type=code&client_id=%s&redirect_uri=%s&state=%s&display=mobile"
//#define OAUTH2_AUTHENTICATION "https://open.weixin.qq.com/connect/oauth2/authorize?appid=%s&redirect_uri=%s&response_type=code&scope=snsapi_base&state=%s#wechat_redirect"
//#define OAUTH2_AUTHENTICATION "https://openapi.baidu.com/social/oauth/2.0/authorize?response_type=code&media_type=qqdenglu&client_id=%s&redirect_uri=%s&state=%s&display=mobile"
//#define OAUTH2_ACCESS_TOKEN "https://api.weixin.qq.com/sns/oauth2/access_token?appid=%s&secret=%s&code=%s&grant_type=authorization_code"
#define OAUTH2_ACCESS_TOKEN "https://graph.qq.com/oauth2.0/token?grant_type=authorization_code&client_id=%s&client_secret=%s&code=%s&redirect_uri=%s"
//#define OAUTH2_ACCESS_TOKEN "https://openapi.baidu.com/social/oauth/2.0/token?grant_type=authorization_code&client_id=%s&client_secret=%s&code=%s&redirect_uri=%s"
#define OAUTH2_GET_OPENID "https://graph.qq.com/oauth2.0/me?access_token=%s"

void OAuth2_process();
void OAuth2_CallBack();

size_t curl_receive(void *buffer, size_t size, size_t nmemb, char *html);

#endif