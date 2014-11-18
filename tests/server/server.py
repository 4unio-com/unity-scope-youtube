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

def read_file(path):
    file = os.path.join(os.path.dirname(__file__), path)
    if os.path.isfile(file):
        with open(file, 'r') as fp:
            content = fp.read()
    else:
        raise Exception("File '%s' not found\n" % file)
    return content

GUIDE_CATEGORIES = read_file('guide-categories.json')

class ErrorHandler(tornado.web.RequestHandler):
    def write_error(self, status_code, **kwargs):
        self.write(json.dumps({'error': '%s: %d' % (kwargs["exc_info"][1], status_code)}))

class Channels(ErrorHandler):
    def get(self):
        validate_header(self, 'Accept-Encoding', 'gzip')
        validate_argument(self, 'part', 'snippet,statistics')

        file = 'channels/%s.json' % self.get_argument('categoryId', None)
        self.write(read_file(file))
        self.finish()

class ChannelSections(ErrorHandler):
    def get(self):
        validate_header(self, 'Accept-Encoding', 'gzip')
        validate_argument(self, 'part', 'contentDetails')

        file = 'channelSections/%s.json' % self.get_argument('channelId', None)
        self.write(read_file(file))
        self.finish()

class GuideCategories(ErrorHandler):
    def get(self):
        validate_header(self, 'Accept-Encoding', 'gzip')
        validate_argument(self, 'part', 'snippet')

        self.write(GUIDE_CATEGORIES)
        self.finish()

class Playlists(ErrorHandler):
    def get(self):
        validate_header(self, 'Accept-Encoding', 'gzip')
        validate_argument(self, 'part', 'snippet,contentDetails')

        file = 'playlists/%s.json' % self.get_argument('channelId', None)
        self.write(read_file(file))
        self.finish()

class PlaylistItems(ErrorHandler):
    def get(self):
        validate_header(self, 'Accept-Encoding', 'gzip')
        validate_argument(self, 'part', 'snippet,contentDetails')

        file = 'playlistItems/%s.json' % self.get_argument('playlistId', None)
        self.write(read_file(file))
        self.finish()

class Search(ErrorHandler):
    def get(self):
        validate_header(self, 'Accept-Encoding', 'gzip')
        validate_argument(self, 'part', 'snippet')
        validate_argument(self, 'type', 'video')

        q = self.get_argument('q', None)
        channelId = self.get_argument('channelId', None)
        videoCategoryId = self.get_argument('videoCategoryId', None)
        if videoCategoryId and q:
            self.write(read_file('search/q/%s%s.json' % ( q, videoCategoryId)))
        elif q:
            self.write(read_file('search/q/%s.json' % q))
        elif channelId:
            self.write(read_file('search/channelId/%s.json' % channelId))

        self.finish()

class Videos(ErrorHandler):
    def get(self):
        validate_header(self, 'Accept-Encoding', 'gzip')
        validate_argument(self, 'part', 'snippet')

        videoCategoryId = self.get_argument('videoCategoryId', None)
        if videoCategoryId:
            self.write(read_file('videos/videoCategoryId/%s.json' % videoCategoryId))

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
        (r"/youtube/v3/channels", Channels),
        (r"/youtube/v3/channelSections", ChannelSections),
        (r"/youtube/v3/guideCategories", GuideCategories),
        (r"/youtube/v3/playlists", Playlists),
        (r"/youtube/v3/playlistItems", PlaylistItems),
        (r"/youtube/v3/search", Search),
    (r"/youtube/v3/videos", Videos),
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
