#pragma once

/*  由于是单线程执行，所以什么都不用考虑。 */
extern FCGX_Request request; //引用外部(URPScoreHelper.h)中定义的FCGI请求上下文。
extern std::string header;
extern std::string footer;
extern int g_QueryCounter;
extern int g_users;

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
void parse_admin_change_password();
void do_admin_change_password();
void parse_admin_adv_card();
void change_admin_adv_card();
void parse_admin_info();
void parse_find_user();
void do_find_user();
void admin_intro();
void homepage_notice();
void set_discussion();
bool session();