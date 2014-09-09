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

    string kind = data["kind"].asString();

    json::Value id = data["id"];
    if (kind == kind_str()) {
        id_ = id.asString();
    } else {
        id_ = id["channelId"].asString();
    }

    json::Value snippet = data["snippet"];
    title_ = snippet["title"].asString();
    description_ = snippet["description"].asString();
    json::Value thumbnails = snippet["thumbnails"];
    json::Value picture = thumbnails["default"];
    picture_ = picture["url"].asString();

    json::Value statistics = data["statistics"];
    subscriber_count_ = stoi(statistics["subscriberCount"].asString());
}

const string & Channel::title() const {
    return title_;
}

const string & Channel::description() const {
    return description_;
}

const string & Channel::picture() const {
    return picture_;
}

const string & Channel::id() const {
    return id_;
}

unsigned int Channel::subscriber_count() const {
    return subscriber_count_;
}

Resource::Kind Channel::kind() const {
    return Resource::Kind::channel;
}

std::string Channel::kind_str() const {
    return "youtube#channel";
}
