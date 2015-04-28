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
 * Author: Kyle Nitzsche<kyle.nitzsche@canonical.com>
 */

#include <youtube/api/subscription-item.h>

#include <iostream>
#include <json/json.h>

namespace json = Json;

using namespace youtube::api;
using namespace std;

SubscriptionItem::SubscriptionItem(const json::Value &data) {


    cout << "==== in subs item construnctor";

    string kind = data["kind"].asString();

    json::Value id = data["id"];
    if (kind == kind_str()) {
        id_ = id.asString();
    } else {
        id_ = id["videoId"].asString();
    }

    Json::Value snippet = data["snippet"];

    title_ = snippet["title"].asString();
    description_ = snippet["description"].asString();
    username_ = snippet["channelTitle"].asString();

    json::Value thumbnails = snippet["thumbnails"];
    json::Value picture = thumbnails["high"];
    picture_ = picture["url"].asString();

    json::Value resourceId = snippet["resourceId"];
    video_id_ = resourceId["videoId"].asString();
    link_ = "http://www.youtube.com/watch?v=" + video_id_;

}

const std::string & SubscriptionItem::title() const {
    return title_;
}

const string & SubscriptionItem::username() const {
    return username_;
}

const std::string & SubscriptionItem::id() const {
    return id_;
}

const std::string & SubscriptionItem::video_id() const {
    return video_id_;
}

const string & SubscriptionItem::link() const {
    return link_;
}

const std::string & SubscriptionItem::picture() const {
    return picture_;
}

const std::string & SubscriptionItem::description() const {
    return description_;
}

Resource::Kind SubscriptionItem::kind() const {
    return Resource::Kind::subscriptionItem;
}

std::string SubscriptionItem::kind_str() const {
    return "youtube#playlistItem";
}
