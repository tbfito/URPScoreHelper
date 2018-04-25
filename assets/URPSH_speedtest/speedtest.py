#!/usr/bin/python
# coding: UTF-8

from flask import Flask, request, Response, render_template
from datetime import datetime
from io import BytesIO
import pycurl, sys, os, time, json, _thread, logging

server_list_response = ''
access_counter = 0
log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)
app = Flask(__name__)

@app.route('/urpsh', methods=['GET', 'POST'])
def http():
    try:
        userIp = request.headers['X-Forwarded-For']
    except:
        userIp = request.remote_addr
    global access_counter
    access_counter = access_counter + 1
    print('[' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') + '] /' + userIp + ' 获取服务器列表... #' + str(access_counter))
    resp = Response(server_list_response, status=200, mimetype='application/json')
    resp.headers['Access-Control-Allow-Origin'] = 'https://urpsh.iedon.com'
    resp.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate'
    resp.headers['Pragma'] = 'no-cache'
    resp.headers['Expires'] = 'Thu, 16 Oct 1997 00:00:00 GMT'
    return resp

def test_speed(input_url):
    buffer = BytesIO()
    c = pycurl.Curl()
    c.setopt(pycurl.SSL_VERIFYPEER, 0)
    c.setopt(c.WRITEDATA, buffer)
    c.setopt(pycurl.ENCODING, 'gzip')
    c.setopt(pycurl.URL,input_url)
    c.setopt(pycurl.MAXREDIRS, 5)
    c.setopt(pycurl.CONNECTTIMEOUT, 3)

    try:
        c.perform()
        http_total_time = c.getinfo(pycurl.TOTAL_TIME)
    except pycurl.error as e:
        http_total_time = -1

    c.close()

    if (buffer.getvalue() == b'\x52\x45\x51\x55\x45\x53\x54\x2D\x46\x41\x49\x4C\x45\x44'):
        http_total_time = -1

    return '%.2f' % http_total_time

def get_server_list():
    # test_speed() 可获取服务器响应时间, -1 为超时宕机。 -2为校园网线路(需要手工添加)
    list = [\
    ["open", "扬州大学", "YZU Public", "https://urpsh.iedon.com/yzu/OAuth2.fcgi", test_speed("https://urpsh.iedon.com/yzu/captcha.fcgi")],\
    ["open", "广院入口", "GLC Public", "https://urpsh.iedon.com/glxy/OAuth2.fcgi", test_speed("https://urpsh.iedon.com/glxy/captcha.fcgi")],\
	#["closed", "扬大测试版", "YZU Beta, Internal", "http://urpsh-intranet.iedon.com/yzu/", "-2"],\
    #["closed", "广院测试版", "GLC Beta, Internal", "http://urpsh-intranet.iedon.com/glxy/", "-2"],\
	["open", "密码自助服务", "Acc. Self-Service", "http://urpsh-selfsvc.iedon.com/selfsvc.php", test_speed("http://home.iedon.com:9287/get_number")]\
    ]
    return list

def get_server_notice():
    list = [\
    ["密码自助服务因系统故障 😂", ""],\
    ["将暂时封闭入口 😭", ""],\
    ["请移步微信群寻求人工服务！", ""],\
    ["谢谢亲的理解 😂", ""],\
    ["ご利用ありがとうございました", ""]\
    ]
    return list

def update_response():
    while True:
        try:
            list = get_server_list()
            length = len(list)
            ret = '{"servers": ['
            for index,value in enumerate(list):
                ret += '{'
                for i,data in enumerate(value):
                    if (i == 0):
                        ret = ret + '"status": "' + data + '", '
                    if (i == 1):
                        ret = ret + '"cn_name": "' + data + '", '
                    if (i == 2):
                        ret = ret + '"en_name": "' + data + '", '
                    if (i == 3):
                        ret = ret + '"url": "' + data + '", '
                    if (i == 4):
                        ret = ret + '"speed": "' + data + '"'
                if (index == length - 1):
                    ret += '}'
                else:
                    ret += '},'
            list = get_server_notice()
            length = len(list)
            ret += '], "notices": ['
            for index,value in enumerate(list):
                ret += '{'
                for i,data in enumerate(value):
                    if (i == 0):
                        ret = ret + '"content": "' + data + '", '
                    if (i == 1):
                        ret = ret + '"link": "' + data + '"'
                if (index == length - 1):
                    ret += '}'
                else:
                    ret += '},'
            ret += ']}'
            global server_list_response
            server_list_response = ret.encode("UTF-8")
        except Exception as e:
            print (e)
            pass
        time.sleep(60)

if __name__ == '__main__':
    try:
        _thread.start_new_thread(update_response, ())
    except:
        print ("[ERROR] Unable to start thread.")
    app.run(port = 8124, host = '0.0.0.0', debug = False)
