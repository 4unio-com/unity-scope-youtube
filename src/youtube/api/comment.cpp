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
 *         Gary Wang  <gary.wang@canonical.com>
 */

#include <youtube/api/comment.h>

#include <json/json.h>

namespace json = Json;
using namespace youtube::api;
using namespace std;

Comment::Comment(const json::Value &data) :
        user_(data["snippet"]["topLevelComment"]["snippet"]) {
    body_ = data["snippet"]["topLevelComment"]["snippet"]["textDisplay"].asString();
    created_at_ = data["snippet"]["topLevelComment"]["publishedAt"].asString();
    
    id_ = data["id"].asString();
}

const string & Comment::id() const {
    return id_; 
}

const string & Comment::body() const {
    return body_;
}

const string & Comment::title() const {
    return user_.title();
}

const string & Comment::picture() const {
    return user_.picture();
}

const string & Comment::created_at() const {
    return created_at_;
}

const User & Comment::user() const {
    return user_;
}

Resource::Kind Comment::kind() const {
    return Resource::Kind::comment;
}

std::string Comment::kind_str() const {
    return "comment";
}
