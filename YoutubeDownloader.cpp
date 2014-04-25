/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 */

#include"YoutubeDownloader.h"
#include<iostream>
#include<memory>
#include<libsoup/soup.h>

const char *urlbase = "https://www.googleapis.com/youtube/v3/search?part=snippet";

struct YoutubeDownloader::Private{
    std::string apikey;

    Private(const std::string &apikey) : apikey(apikey) {
    }
};

YoutubeDownloader::YoutubeDownloader(const std::string &s) : p(new Private(s)) {

}

YoutubeDownloader::~YoutubeDownloader() {
    delete p;
}

void YoutubeDownloader::query(const std::string &q) {
    std::unique_ptr<SoupSession, void(*)(gpointer)> session(soup_session_sync_new(), g_object_unref);
    if(!session) {
        throw std::runtime_error("Could not create Soup session.");
    }
    std::string url(urlbase);
    url += "&q=" + q; // FIXME: quote input
    url += "&key=" + p->apikey;
    std::unique_ptr<SoupMessage, void(*)(gpointer)> msg(soup_message_new("GET", url.c_str()), g_object_unref);
    std::cout << "Downloading url " << url << std::endl;
    guint status = soup_session_send_message(session.get(), msg.get());
    if(!SOUP_STATUS_IS_SUCCESSFUL(status)) {
        std::cerr << "Download failed." << std::endl;
    }
    std::string result(msg->response_body->data, msg->response_body->length);
    std::cout << "\nGot this result:\n" << result << std::endl;
}
