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

#include <youtube/api/client.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <json/json.h>

#include <iostream>

namespace http = core::net::http;
namespace json = Json;
namespace net = core::net;

using namespace youtube::api;
using namespace std;

Client::Client(Config::Ptr config) :
        config_(config), cancelled_(false) {
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
static deque<shared_ptr<T>> get_list(const json::Value &root) {
    deque<shared_ptr<T>> results;
    json::Value data = root["items"];
    for (json::ArrayIndex index = 0; index < data.size(); ++index) {
        results.push_back(make_shared<T>(data[index]));
    }
    return results;
}

Client::VideoList Client::videos(const string &query) {
    json::Value root;
    get( { "youtube", "v3", "search" }, { { "type", "video" }, { "part",
            "snippet" }, { "maxResults", "10" }, { "q", query } }, root);
    return get_list<Video>(root);
}

Client::VideoCategoryList Client::video_categories() {
    json::Value root;
    // FIXME Get the real country code
    string country_code = "US";
    get( { "youtube", "v3", "videoCategories" }, { { "part", "snippet" }, {
            "regionCode", country_code } }, root);
    return get_list<VideoCategory>(root);
}

Client::VideoList Client::category_videos(const string &category) {
    json::Value root;
    get( { "youtube", "v3", "videos" }, { { "part", "snippet" }, { "chart",
            "mostPopular" }, { "videoCategoryId", category } }, root);
    return get_list<Video>(root);
}

Client::VideoList Client::feed() {
    json::Value root;
    get( { "youtube", "v3", "videos" }, { { "part", "snippet" }, { "maxResults",
            "10" }, { "chart", "mostPopular" } }, root);
    return get_list<Video>(root);
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
