#!/usr/bin/python
# coding: UTF-8
import pycurl,sys,os,time,json
 
class idctest:
    def __init__(self):
        self.contents = ''
    def body_callback(self,buf):
        self.contents = ''

def test_speed(input_url):
    t = idctest()
    c = pycurl.Curl()
    c.setopt(pycurl.SSL_VERIFYPEER, 0)
    c.setopt(pycurl.WRITEFUNCTION, t.body_callback)
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

    return '%.2f' % http_total_time

def server_list():
    list = [\
    ["open", "扬大 (公网)", "YZU Public", "https://urpsh.iedon.com/yzu/OAuth2.fcgi", test_speed("https://urpsh.iedon.com/yzu/captcha.fcgi")],\
    ["open", "扬大 (内测)", "YZU Beta, Internal", "http://urpsh-internal.iedon.com/yzu/", "校园网"],\
    ["open", "广陵 (公网)", "GLC Public", "https://urpsh.iedon.com/glxy/OAuth2.fcgi", test_speed("https://urpsh.iedon.com/glxy/captcha.fcgi")],\
    ["open", "广陵 (内测)", "GLC Beta, Internal", "http://urpsh-internal.iedon.com/glxy/", "校园网"]\
    ]
    return list
	
def run():
    list = server_list()
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
    ret += ']}'
    f = open('server.json', 'wb+')
    f.write(ret.encode("UTF-8"))
    f.close()

if __name__ == '__main__':

    while True:
        try:
            run()
        except Exception as e:
            print (e)
            pass

        time.sleep(60)
