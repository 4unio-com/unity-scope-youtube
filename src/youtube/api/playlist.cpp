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

#include <youtube/api/playlist.h>

#include <iostream>
#include <json/json.h>

namespace json = Json;

using namespace youtube::api;
using namespace std;

Playlist::Playlist(const json::Value &data) {
    string kind = data["kind"].asString();

    json::Value snippet = data["snippet"];

    name_ = snippet["title"].asString();

    json::Value id = data["id"];
    if (kind == "youtube#playlist") {
        id_ = id.asString();
    } else {
        id_ = id["playlistId"].asString();
    }

    json::Value thumbnails = snippet["thumbnails"];
    json::Value picture = thumbnails["default"];
    picture_ = picture["url"].asString();

    json::Value content_details = data["contentDetails"];
    item_count_ = content_details["itemCount"].asInt();
}

const std::string & Playlist::title() const {
    return name_;
}

const std::string & Playlist::picture() const {
    return picture_;
}

const std::string & Playlist::id() const {
    return id_;
}

int Playlist::item_count() const {
    return item_count_;
}

Resource::Kind Playlist::kind() const {
    return Resource::Kind::playlist;
}
