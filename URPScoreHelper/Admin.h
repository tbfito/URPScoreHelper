#pragma once

/*  由于是单线程执行，所以什么都不用考虑，爽歪歪。 */
extern FCGX_Request request; //引用外部(URPScoreHelper.h)中定义的FCGI请求上下文。
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