from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
logger = logging.getLogger(__name__)


class ESP8266(object):

    def __str__(self):
        return "ESP8266_left_{}_right{}".format(self._left, self._right)

    def __init__(self, left="", right=""):

        self._left = left
        self._right = right

        self._gpio2 = 0
        self._gpio15 = 0

        # ans = requests.get(SERVICE_URL + '?q="%s"' % text)
        print(self)

    def handle_request(self, request_):
        return {'new_value': float(request_.args.get('value'))+1}
