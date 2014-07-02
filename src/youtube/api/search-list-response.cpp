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
#include <youtube/api/playlist.h>
#include <youtube/api/search-list-response.h>
#include <youtube/api/video.h>
#include <youtube/api/video-category.h>

#include <iostream>
#include <unordered_map>

#include <json/json.h>

namespace json = Json;

using namespace youtube::api;
using namespace std;

namespace {
static unordered_map<string, function<Resource::Ptr(const json::Value &)>> TYPES =
        { { "youtube#videoCategory", [](const json::Value &value) {
            return make_shared<VideoCategory>(value);
        } }, { "youtube#video", [](const json::Value &value) {
            return make_shared<Video>(value);
        } }, { "youtube#channel", [](const json::Value &value) {
            return make_shared<Channel>(value);
        } }, { "youtube#playlist", [](const json::Value &value) {
            return make_shared<Playlist>(value);
        } } };
}

SearchListResponse::SearchListResponse(const json::Value &data) {
    json::Value page_info = data["pageInfo"];

    total_results_ = page_info["totalResults"].asInt();

    json::Value items = data["items"];
    for (json::ArrayIndex index = 0; index < items.size(); ++index) {
        json::Value item = items[index];
        string kind = item["kind"].asString();
        if (kind == "youtube#searchResult") {
            kind = item["id"]["kind"].asString();
        }
        const auto f = TYPES.find(kind);
        if (f == TYPES.cend()) {
            cerr << "Couldn't create type: " << kind << endl;
            cerr << item.toStyledString() << endl;
            cerr << "------------------" << endl;
        } else {
            items_.emplace_back(f->second(item));
        }
    }
}

SearchListResponse::ResourceList SearchListResponse::items() {
    return items_;
}

std::size_t SearchListResponse::total_results() const {
    return total_results_;
}
