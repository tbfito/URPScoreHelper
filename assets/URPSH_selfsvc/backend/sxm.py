#!/usr/bin/python
# coding: UTF-8

from flask import Flask, request, Response, render_template
from datetime import datetime
from io import BytesIO
from urllib import parse
import pycurl, sys, os, time, json, _thread, logging, random


random_number = 0
log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)
app = Flask(__name__)
 
 
@app.route('/get_number', methods=['GET', 'POST'])
def http_get_number():
    try:
        userIp = request.headers['X-Forwarded-For']
    except:
        userIp = request.remote_addr
    print('[' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') + '] /' + userIp + ' 获取当前时效码：#' + str(random_number))
    resp = Response(str(random_number), status=200, mimetype='text/plain')
    resp.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate'
    resp.headers['Pragma'] = 'no-cache'
    resp.headers['Expires'] = 'Thu, 16 Oct 1997 00:00:00 GMT'
    return resp

def get_voice(input_url):
    buffer = BytesIO()
    c = pycurl.Curl()
    c.setopt(pycurl.SSL_VERIFYPEER, 0)
    c.setopt(c.WRITEDATA, buffer)
    c.setopt(pycurl.URL,input_url)
    c.setopt(pycurl.MAXREDIRS, 5)
    c.setopt(pycurl.CONNECTTIMEOUT, 5)

    try:
        c.perform()
        http_total_time = c.getinfo(pycurl.TOTAL_TIME)
    except pycurl.error as e:
        print (e)

    c.close()

    return buffer

	
def update_random():
    while True:
        try:
            global random_number
            random_number = random.randint(100000, 999999)
            text = '您好，感谢使用艾神的自助密码找回服务。为响应国家互联网应用实名政策，我们会收集您的实名信息。时效验证码为六位数字。当前的时效验证码为：' + str(random_number) + '。' + str(random_number) + '。' + str(random_number) + '。时效验证码会很快失效，请尽快使用。谢谢您的使用，再见。如果您还有任何问题，按4语音留言。'
            text = parse.quote(text)
            url = 'http://tts.baidu.com/text2audio?idx=1&tex={' + text + '}&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=4&per=0&vol=5&pit=5'
            with open("random.mp3", 'wb') as f:
                f.write(get_voice(url).getvalue())
                f.flush()
                f.close()
        except Exception as e:
            print (e)
            pass
        time.sleep(600)

if __name__ == '__main__':
    try:
        _thread.start_new_thread(update_random, ())
    except:
        print ("[ERROR] Unable to start thread.")
    app.run(port = 8123, host = '0.0.0.0', debug = False)
