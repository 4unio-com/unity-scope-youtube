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
#include <youtube/api/client.h>
#include <youtube/api/playlist.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <json/json.h>

#include <iostream>
#include <unordered_map>

namespace http = core::net::http;
namespace json = Json;
namespace net = core::net;

using namespace youtube::api;
using namespace std;

Client::Client(Config::Ptr config, int cardinality, const string& locale) :
        config_(config), cardinality_(cardinality), locale_(locale), cancelled_(
        false) {
}

static string make_uri(const string host, const deque<string> &endpoints,
        const vector<pair<string, string>> &querys,
        shared_ptr<http::Client> client) {
    string uri = host;
    for (const string &endpoint : endpoints) {
        uri.append("/" + endpoint);
    }
    bool first = true;
    for (auto it : querys) {
        if (first) {
            uri.append("?");
            first = false;
        } else {
            uri.append("&");
        }
        uri.append(client->url_escape(it.first));
        uri.append("=");
        uri.append(client->url_escape(it.second));
    }
    return uri;
}

void Client::get(const deque<string> &path,
        const vector<pair<string, string>> &parameters, json::Value &root) {
    cancelled_ = false;

    auto client = http::make_client();

    http::Request::Configuration configuration;
    vector<pair<string, string>> complete_parameters(parameters);
    if (cardinality_ > 0) {
        complete_parameters.push_back(
                { "maxResults", to_string(cardinality_) });
    }
    if (config_->authenticated) {
        configuration.header.add("Authorization",
                "bearer " + config_->access_token);
    } else {
        complete_parameters.push_back( { "key", config_->api_key });
    }
    configuration.uri = make_uri(config_->apiroot, path, complete_parameters,
            client);
    configuration.header.add("Accept", config_->accept);
    configuration.header.add("User-Agent", config_->user_agent);

    auto request = client->head(configuration);

    try {
        auto response = request->execute(
                bind(&Client::progress_report, this, placeholders::_1));

        json::Reader reader;
        reader.parse(response.body, root);

        if (response.status != http::Status::ok) {
            cerr << "ERROR: " << response.body << endl;
            throw domain_error(root["error"].asString());
        }
    } catch (net::Error &) {
    }
}

template<typename T>
static deque<shared_ptr<T>> get_typed_list(const string &filter,
        const json::Value &root) {
    deque<shared_ptr<T>> results;
    json::Value data = root["items"];
    for (json::ArrayIndex index = 0; index < data.size(); ++index) {
        json::Value item = data[index];

        string kind = item["kind"].asString();
        if (kind == "youtube#searchResult") {
            kind = item["id"]["kind"].asString();
        }

        if (kind == filter) {
            results.push_back(make_shared<T>(item));
        }
    }
    return results;
}

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

static Client::ResourceList get_list(const json::Value &root) {
    Client::ResourceList results;
    json::Value data = root["items"];
    for (json::ArrayIndex index = 0; index < data.size(); ++index) {
        json::Value item = data[index];
        string kind = item["kind"].asString();
        if (kind == "youtube#searchResult") {
            kind = item["id"]["kind"].asString();
        }
        auto f = TYPES.find(kind);
        if (f == TYPES.end()) {
            cerr << "Couldn't create type: " << kind << endl;
            cerr << item.toStyledString() << endl;
            cerr << "------------------" << endl;
        } else {
            results.push_back(TYPES[kind](item));
        }
    }
    return results;
}

Client::ResourceList Client::search(const string &query) {
    json::Value root;
    get( { "youtube", "v3", "search" }, { { "part", "snippet" }, { "maxResults",
            "10" }, { "q", query } }, root);
    return get_list(root);
}

Client::VideoCategoryList Client::video_categories() {
    json::Value root;
    // FIXME Get the real country code
    string country_code = "US";
    get( { "youtube", "v3", "videoCategories" }, { { "part", "snippet" }, {
            "regionCode", country_code }, { "h1", locale_ } }, root);
    return get_typed_list<VideoCategory>("youtube#videoCategory", root);
}

Client::GuideCategoryList Client::guide_categories() {
    json::Value root;
    // FIXME Get the real country code
    string country_code = "US";
    get( { "youtube", "v3", "guideCategories" }, { { "part", "snippet" }, {
            "regionCode", country_code }, { "h1", locale_ } }, root);
    return get_typed_list<GuideCategory>("youtube#guideCategory", root);
}

Client::ChannelList Client::category_channels(const string &categoryId) {
    json::Value root;
    get( { "youtube", "v3", "channels" },
            { { "part", "snippet,contentDetails" }, { "categoryId", categoryId } },
            root);
    return get_typed_list<Channel>("youtube#channel", root);
}

Client::VideoList Client::channel_videos(const string &channel) {
    json::Value root;
    get( { "youtube", "v3", "search" }, { { "part", "snippet" }, { "type",
            "video" }, { "order", "viewCount" }, { "channelId", channel } },
            root);
    return get_typed_list<Video>("youtube#video", root);
}

Client::VideoList Client::playlist_videos(const string &playlist) {
    json::Value root;
    get( { "youtube", "v3", "search" }, { { "part", "snippet" }, { "type",
            "video" }, { "order", "viewCount" }, { "playlistId", playlist } },
            root);
    return get_typed_list<Video>("youtube#video", root);
}

Client::ResourceList Client::feed() {
    json::Value root;
    get( { "youtube", "v3", "videos" }, { { "part", "snippet" }, { "chart",
            "mostPopular" } }, root);
    return get_list(root);
}

http::Request::Progress::Next Client::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
            http::Request::Progress::Next::abort_operation :
            http::Request::Progress::Next::continue_operation;
}

void Client::cancel() {
    cancelled_ = true;
}

Config::Ptr Client::config() {
    return config_;
}
