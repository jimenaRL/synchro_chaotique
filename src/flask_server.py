# !flask/bin/python
import sys
import json
import argparse

from esp8266 import ESP8266

from flask import Flask, request


parser = argparse.ArgumentParser()
parser.add_argument("-ip", help="ip", type=str, default='0.0.0.0')
parser.add_argument("-port", help="port", type=int, default=5213)
parser.add_argument("-debug", help="debug mode", type=int, default=0)
parser.add_argument("-left", help="ip left", type=int, default=666)
parser.add_argument("-right", help="ip  right", type=int, default=777)
pargs = parser.parse_args()

_esp8266 = ESP8266(pargs.left, pargs.right)

app = Flask(__name__)


@app.route('/')
def default():
    print(request)
    return {
        "language": "unknown",
        "status": 400,
        "message": "Missing endpoint. Use '/GPIO2/' or '/GPIO15/' endpoint.",
    }


@app.route('/gpio2/', methods=['GET'])
def gpio2():
    try:
        response = _esp8266.handle_request(request)
        return json.dumps(response)

    except Exception as ex:
        exc_type, exc_obj, exc_tb = sys.exc_info()
        return {
            "status": 300,
            "message": "Error on server side at line {}: {}".format(exc_tb.tb_lineno, ex),
        }

app.run(pargs.ip, pargs.port, pargs.debug)
