#pragma once

/*  �����ǵ��߳�ִ�У�����ʲô�����ÿ��ǣ�ˬ���ᡣ */
extern FCGX_Request request; //�����ⲿ(URPScoreHelper.h)�ж����FCGI���������ġ�
extern std::string header;
extern std::string footer;

void parse_admin_login();
void do_admin_login();
void admin_error(const char *err_msg);
std::string generate_session();
bool verify_session();
void parse_admin_index();
void parse_admin_settings();
void save_admin_settings();
std::string _POST(std::string & post, const char *name);
void UpdateSettings(const char *name, const char *value);
void decode_post_data(std::string & str);