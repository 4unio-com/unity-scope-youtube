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

#include <youtube/api/channel-section.h>

#include <iostream>
#include <json/json.h>

namespace json = Json;

using namespace youtube::api;
using namespace std;

ChannelSection::ChannelSection(const json::Value &data) {
    string kind = data["kind"].asString();

    json::Value id = data["id"];
    if (kind == kind_str()) {
        id_ = id.asString();
    } else {
        id_ = id["channelSectionId"].asString();
    }

    json::Value contentDetails = data["contentDetails"];
    json::Value playlists = contentDetails["playlists"];

    playlist_id_ = playlists.get(json::ArrayIndex(0), "").asString();
}

const string & ChannelSection::title() const {
    return id_;
}

const string & ChannelSection::picture() const {
    return id_;
}

const string & ChannelSection::id() const {
    return id_;
}

const string & ChannelSection::playlist_id() const {
    return playlist_id_;
}

Resource::Kind ChannelSection::kind() const {
    return Resource::Kind::channelSection;
}

std::string ChannelSection::kind_str() const {
    return "youtube#channelSection";
}
