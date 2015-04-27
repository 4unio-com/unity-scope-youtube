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
 * Author: Kyle Nitzsche <kyle.nitzsche@canonical.com>
 */

#include <youtube/api/subscription.h>

#include <iostream>
#include <json/json.h>

namespace json = Json;

using namespace youtube::api;
using namespace std;

Subscription::Subscription(const json::Value &data) {

    json::Value snippet = data["snippet"];
    title_ = snippet["title"].asString();
    json::Value resourceId = snippet["resourceId"];
    id_ = resourceId["channelId"].asString();
}

const string & Subscription::title() const {
    return title_;
}

const string & Subscription::picture() const {
    return picture_;
}

const string & Subscription::id() const {
    return id_;
}

Resource::Kind Subscription::kind() const {
    return Resource::Kind::subscription;
}

std::string Subscription::kind_str() const {
    return "youtube#subscription";
}
