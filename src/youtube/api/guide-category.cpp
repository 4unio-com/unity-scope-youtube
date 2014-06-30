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

#include <youtube/api/guide-category.h>

#include <iostream>
#include <json/json.h>

namespace json = Json;

using namespace youtube::api;
using namespace std;

GuideCategory::GuideCategory(const json::Value &data) {
    id_ = data["id"].asString();

    json::Value snippet = data["snippet"];
    name_ = snippet["title"].asString();
}

const std::string & GuideCategory::title() const {
    return name_;
}

const std::string & GuideCategory::picture() const {
    return picture_;
}

const std::string & GuideCategory::id() const {
    return id_;
}

Resource::Kind GuideCategory::kind() const {
    return Resource::Kind::guideCategory;
}
