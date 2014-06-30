/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <youtube/api/video.h>

#include <json/json.h>

namespace json = Json;
using namespace youtube::api;
using namespace std;

Video::Video(const json::Value &data) {
    string kind = data["kind"].asString();

    json::Value snippet = data["snippet"];

    title_ = snippet["title"].asString();
    description_ = snippet["description"].asString();

    json::Value id = data["id"];
    if (kind == "youtube#video") {
        id_ = id.asString();
    } else {
        id_ = id["videoId"].asString();
    }

    link_ = "https://www.youtube.com/watch?v=" + id_;

    username_ = snippet["channelTitle"].asString();

    json::Value thumbnails = snippet["thumbnails"];
    json::Value picture = thumbnails["high"];
    picture_ = picture["url"].asString();
}

const string & Video::title() const {
    return title_;
}

const string & Video::username() const {
    return username_;
}

const string & Video::id() const {
    return id_;
}

const string & Video::link() const {
    return link_;
}

const string & Video::picture() const {
    return picture_;
}

const string & Video::description() const {
    return description_;
}

Resource::Kind Video::kind() const {
    return Resource::Kind::video;
}
