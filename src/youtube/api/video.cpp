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

    name_ = snippet["title"].asString();
    description_ = snippet["description"].asString();

    json::Value id = data["id"];
    string video_id;
    if (kind == "youtube#video") {
        video_id = id.asString();
    } else {
        video_id = id["videoId"].asString();
    }
    uri_ = "https://www.youtube.com/watch?v=" + video_id;

    username_ = snippet["channelTitle"].asString();

    json::Value thumbnails = snippet["thumbnails"];
    json::Value picture = thumbnails["high"];
    picture_ = picture["url"].asString();
}

const std::string & Video::name() const {
    return name_;
}

const std::string & Video::username() const {
    return username_;
}

const std::string & Video::uri() const {
    return uri_;
}

const std::string & Video::picture() const {
    return picture_;
}

const std::string & Video::description() const {
    return description_;
}
