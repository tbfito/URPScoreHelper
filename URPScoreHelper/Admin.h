﻿#pragma once

#ifndef __ADMIN_H__

	#define __ADMIN_H__

	extern FCGX_Request request;
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
	void site_maintenance();
	void set_oauth2();

#endif