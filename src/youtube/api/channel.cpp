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

#include <youtube/api/channel.h>

#include <iostream>
#include <json/json.h>

namespace json = Json;

using namespace youtube::api;
using namespace std;

Channel::Channel(const json::Value &data) {
//    cerr << data.toStyledString() << endl;

    string kind = data["kind"].asString();

    json::Value id = data["id"];
    if (kind == "youtube#channel") {
        id_ = id.asString();
    } else {
        id_ = id["channelId"].asString();
    }

    json::Value snippet = data["snippet"];
    name_ = snippet["title"].asString();
    json::Value thumbnails = snippet["thumbnails"];
    json::Value picture = thumbnails["default"];
    picture_ = picture["url"].asString();
}

const std::string & Channel::title() const {
    return name_;
}

const std::string & Channel::picture() const {
    return picture_;
}

const std::string & Channel::id() const {
    return id_;
}

Resource::Kind Channel::kind() const {
    return Resource::Kind::channel;
}
