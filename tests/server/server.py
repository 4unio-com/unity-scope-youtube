#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2014 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Pete Woods <pete.woods@canonical.com>

import base64
import json
import os
import tornado.httpserver
import tornado.ioloop
import tornado.netutil
import tornado.web
import sys

ACCESS_TOKEN = 'the_access_token'

AUTHORIZATION_BEARER = 'Bearer %s' % ACCESS_TOKEN

QUERY_SEARCH = open(os.path.join(os.path.dirname(__file__), 'search.json')).read()

class ErrorHandler(tornado.web.RequestHandler):
    def write_error(self, status_code, **kwargs):
        self.write(json.dumps({'error': '%s: %d' % (kwargs["exc_info"][1], status_code)}))

class Search(ErrorHandler):
    def get(self):
        validate_header(self, 'Accept-Encoding', 'gzip')
        validate_argument(self, 'part', 'snippet')
        validate_argument(self, 'type', 'video')
        validate_argument(self, 'maxResults', '10')

        query = self.get_argument('q', '')
    
        if query == 'banana':
            self.write(QUERY_SEARCH)
        else:
            raise Exception("Unknown query '%s'" % query)
        self.finish()

def validate_argument(self, name, expected):
    actual = self.get_argument(name, '')
    if actual != expected:
        raise Exception("Argument '%s' == '%s' != '%s'" % (name, actual, expected))

def validate_header(self, name, expected):
    actual = self.request.headers.get(name, '')
    if actual != expected:
        raise Exception("Header '%s' == '%s' != '%s'" % (name, actual, expected))

def new_app():
    application = tornado.web.Application([
        (r"/youtube/v3/search", Search),
    ], gzip=True)
    sockets = tornado.netutil.bind_sockets(0, '127.0.0.1')
    server = tornado.httpserver.HTTPServer(application)
    server.add_sockets(sockets)

    sys.stdout.write('%d\n' % sockets[0].getsockname()[1])
    sys.stdout.flush()

    return application

if __name__ == "__main__":
    application = new_app()
    tornado.ioloop.IOLoop.instance().start()
