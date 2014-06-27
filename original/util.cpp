/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 */
#include "util.h"
#include<json/json.h>

std::vector<YoutubeResult> parse_json(const std::string &input) {
    Json::Value root;
    Json::Reader reader;
    std::vector<YoutubeResult> result;
    if(not reader.parse(input, root)) {
        std::cerr << "Json parsing failed." << std::endl;
        return result;
    }
    const Json::Value items = root["items"];
    for(unsigned int i=0; i<items.size(); i++) {
        YoutubeResult yr;
        const Json::Value snippet = items[i]["snippet"];
        yr.title = snippet["title"].asString();
        yr.description = snippet["description"].asString();
        yr.screenshot = snippet["thumbnails"]["high"]["url"].asString();
        result.push_back(yr);
    }
    return result;
}
