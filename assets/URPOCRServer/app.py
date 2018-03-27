#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Date    : 2017-07-22 23:05:28
# @Author  : iEdon & mayongze
# @Link    : https://github.com/iedon & https://github.com/mayongze
# @Version : $Id$

from flask import Flask, request, Response, render_template
import sys
from binascii import a2b_base64
import ctypes,os,hashlib
import logging
from datetime import datetime

log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)

app = Flask(__name__)
dll = None
dll2 = None

@app.route('/', methods=['GET', 'POST'])
def home():
    return "ERROR: Directly access is NOT allowed."

def init():
    global dll
    global dll2
    path = os.getcwd()
    dll = ctypes.windll.LoadLibrary(path + '\\WmCode.dll')
    dll2 = ctypes.windll.LoadLibrary(path + '\\WmCode2.dll')
    os.chdir(path)
    if dll.UseUnicodeString(1,1) and dll.LoadWmFromFile(path + '\\Guangling.dat','iedon') and dll.SetWmOption(7,-1) and dll.SetWmOption(4,1) and dll.SetWmOption(6,80):
        print('init success! [dll1]')
    else:
        print('init error! [dll1]')
        os._exit(0)
    if dll2.UseUnicodeString(1,1) and dll2.LoadWmFromFile(path + '\\Yangda.dat','iedon') and dll2.SetWmOption(7,-1) and dll2.SetWmOption(4,1) and dll2.SetWmOption(6,80):
        print('init success! [dll2]')
    else:
        print('init error! [dll2]')
        os._exit(0)
init()

@app.route('/captcha', methods=['POST'])
def captcha():
    referer = request.headers['Referer']
    if(referer.startswith("https://urpsh.iedon.com/yzu/")):
        # 需要从request对象读取表单内容：
        stream = request.form.get("pic").split("data:image/jpg;base64,")[1]
        #userIp = request.remote_addr
        userIp = request.headers['X-Forwarded-For']
        #print('-------------------------------------------------------------------------------')
        sys.stdout.write('[扬大] ' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') + ' 正在为 ' + userIp + ' 识别验证码... (')
        str = ctypes.create_string_buffer(16) #创建文本缓冲区
        imgbin = a2b_base64(stream)
        if(dll2.GetImageFromBuffer(imgbin, len(imgbin), str)):#使用绝对路径
            code = str.raw.decode("gbk").split('\x00')[0]
            #print('GetVcode Success:%s' % (code,))
            # 将明显识别错误的验证码图片保存下来
            if len(code) != 4:
                #print('GetVcode Fail!')
                #imgName = 'failed/%s-%s.jpg' % (code,hashlib.md5(imgbin).hexdigest())
                #with open(imgName, 'wb') as f:
                #    f.write(imgbin)
                #    f.flush()
                #    f.close()
                code = '__ERROR'
        else:
            code = '__ERROR'
            #print('Function Fail!')
        print(code + ')')
        resp = Response(code, status=200, mimetype='text/plain')
        resp.headers['Access-Control-Allow-Origin'] = 'https://urpsh.iedon.com'
        return resp
    else:
        if(referer.startswith("https://urpsh.iedon.com/glxy/")):
        # 需要从request对象读取表单内容：
            stream = request.form.get("pic").split("data:image/jpg;base64,")[1]
            #userIp = request.remote_addr
            userIp = request.headers['X-Forwarded-For']
            #print('-------------------------------------------------------------------------------')
            sys.stdout.write('[广陵] ' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') + ' 正在为 ' + userIp + ' 识别验证码... (')
            str = ctypes.create_string_buffer(16) #创建文本缓冲区
            imgbin = a2b_base64(stream)
            if(dll.GetImageFromBuffer(imgbin, len(imgbin), str)):#使用绝对路径
                code = str.raw.decode("gbk").split('\x00')[0]
                #print('GetVcode Success:%s' % (code,))
                # 将明显识别错误的验证码图片保存下来
                if len(code) != 4:
                    #print('GetVcode Fail!')
                    #imgName = 'failed/%s-%s.jpg' % (code,hashlib.md5(imgbin).hexdigest())
                    #with open(imgName, 'wb') as f:
                    #    f.write(imgbin)
                    #    f.flush()
                    #    f.close()
                    code = '__ERROR'
            else:
                code = '__ERROR'
                #print('Function Fail!')
            print(code + ')')
            resp = Response(code, status=200, mimetype='text/plain')
            resp.headers['Access-Control-Allow-Origin'] = 'https://urpsh.iedon.com'
            return resp
        else:
            code = 'Access Denied'
            resp = Response(code, status=200, mimetype='text/plain')
            resp.headers['Access-Control-Allow-Origin'] = 'https://urpsh.iedon.com'
            return resp

if __name__ == '__main__':
    print("Listening on port 8123...")
    app.run(port = 8123, host = '0.0.0.0', debug = False)
