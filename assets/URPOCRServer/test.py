#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import requests
import random
from binascii import b2a_base64
#.replace("\\n'", "")

if __name__ == '__main__':
    for i in range(500):
        r = requests.get('http://58.220.248.249:9100/validateCodeAction.do?random=%s' % random.random(),stream=True)
        image = "data:image/jpg;base64," + str(b2a_base64(r.content, newline=False)).replace("b'", "")
        headers = {
                'Connection': 'Keep-Alive',
                'Expect': '100-continue',
                'User-Agent': 'ben',
				'Content-Type' : 'application/x-www-form-urlencoded',
            }
        payload = {'pic':image}
        r = requests.post('http://127.0.0.1:808/captcha',data=payload)
        print(str(i) + " - " + r.text)