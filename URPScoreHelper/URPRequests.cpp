#include "stdafx.h"
#include "URPRequests.h"

const char * REQUEST_HOME_PAGE = "/";
const char * REQUEST_CAPTCHA = "/validateCodeAction.do?random=0.%d";
const char * REQUEST_LOGIN = "/loginAction.do";
const char * REQUEST_QUERY_SCORE = "/bxqcjcxAction.do";
const char * REQUEST_LOGOUT = "/logout.do";
const char * REQUEST_GET_REGISTER_INTERFACE = "/dzzcAction.do";
const char * REQUEST_POST_REGISTER_INTERFACE = "/dzzcAction.do?zc=zc&zxjxjhh=%s";
const char * REQUEST_SET_REPORT_PARAMS = "/setReportParams";
const char * REFERER_REQUEST_REPORT_FILES = "/reportFiles/cj/cj_zwcjd.jsp";
const char * REQUEST_REPORT_FILES = "/reportFiles/cj/cj_zwcjd.jsp?&reportParamsId=%s";
const char * REFERER_REQUEST_TXT_SCORES = "/reportFiles/cj/cj_zwcjd.jsp?&reportParamsId=%s";
const char * REQUEST_TXT_SCORES = "/servlet/com.runqian.report.view.text.TextFileServlet?%s";
const char * REQUEST_PHOTO = "/xjInfoAction.do?oper=img";
const char * REQUEST_TOP = "/menu/top.jsp";
const char * GET_GRADE_BY_QBINFO = "/gradeLnAllAction.do?type=ln&oper=qbinfo";
const char * GET_GRADE_BY_PLAN = "/gradeLnAllAction.do?type=ln&oper=fainfo";
const char * GET_GRADE_BY_FAILED = "/gradeLnAllAction.do?type=ln&oper=bjg";
const char * GET_SMALL_TEST_SCORE = "/cjSearchAction.do?oper=getKscjList";
const char * GET_TEACH_EVAL_LIST = "/jxpgXsAction.do?oper=listWj&pageSize=200";
const char * POST_TEACH_EVAL = "/jxpgXsAction.do?oper=wjpg";
const char * POST_PRE_TEACH_EVAL = "/jxpgXsAction.do";
const char * REQ_CHANGE_PASSWORD = "/modifyPassWordAction.do?pwd=%s";
const char * REQUEST_SCHEDULE = "/xkAction.do?actionType=6";