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

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <core/net/error.h>
#include <core/net/uri.h>
#include <core/net/http/content_type.h>
#include <core/net/http/client.h>
#include <core/net/http/request.h>
#include <core/net/http/response.h>
#include <json/json.h>

#include <iostream>

namespace http = core::net::http;
namespace json = Json;
namespace io = boost::iostreams;
namespace net = core::net;

using namespace youtube::api;
using namespace std;

namespace {

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
            results.emplace_back(make_shared<T>(item));
        }
    }
    return results;
}

}

class Client::Priv {
public:
    Priv(std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client) :
            client_(http::make_client()), worker_ { [this]() {client_->run();} },
            oa_client_(oa_client), cancelled_(false) {
    }

    ~Priv() {
        client_->stop();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    std::shared_ptr<core::net::http::Client> client_;

    std::thread worker_;

    Config config_;
    std::mutex config_mutex_;
    std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client_;

    std::atomic<bool> cancelled_;

    void get(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            http::Request::Handler &handler) {

        http::Request::Configuration configuration;
        net::Uri::QueryParameters complete_parameters(parameters);
        if (config_.authenticated) {
            configuration.header.add("Authorization",
                    "Bearer " + config_.access_token);
        } else {
            complete_parameters.emplace_back("key", config_.api_key);
        }

        net::Uri uri = net::make_uri(config_.apiroot, path,
                complete_parameters);
        configuration.uri = client_->uri_to_string(uri);
        configuration.header.add("Accept", config_.accept);
        configuration.header.add("User-Agent", config_.user_agent + " (gzip)");
        configuration.header.add("Accept-Encoding", "gzip");

        auto request = client_->head(configuration);
        request->async_execute(handler);
    }

    http::Request::Progress::Next progress_report(
            const http::Request::Progress&) {
        return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
    }

    template<typename T>
    future<T> async_get(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            const function<T(const json::Value &root)> &func) {
        auto prom = make_shared<promise<T>>();

        http::Request::Handler handler;
        handler.on_progress(
                bind(&Client::Priv::progress_report, this, placeholders::_1));
        handler.on_error([prom](const net::Error& e)
        {
            prom->set_exception(make_exception_ptr(e));
        });
        handler.on_response(
                [prom,func](const http::Response& response)
                {
                    string decompressed;

                    if(!response.body.empty()) {
                        try {
                            io::filtering_ostream os;
                            os.push(io::gzip_decompressor());
                            os.push(io::back_inserter(decompressed));
                            os << response.body;
                            boost::iostreams::close(os);
                        } catch(io::gzip_error &e) {
                            prom->set_exception(make_exception_ptr(e));
                            return;
                        }
                    }

                    json::Value root;
                    json::Reader reader;
                    reader.parse(decompressed, root);

                    if (response.status != http::Status::ok) {
                        prom->set_exception(make_exception_ptr(domain_error(root["error"].asString())));
                    } else {
                        prom->set_value(func(root));
                    }
                });

        get(path, parameters, handler);

        return prom->get_future();
    }

    bool authenticated() {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return config_.authenticated;
    }

    void update_config() {
        std::lock_guard<std::mutex> lock(config_mutex_);

        if (getenv("YOUTUBE_SCOPE_APIROOT")) {
            config_.apiroot = getenv("YOUTUBE_SCOPE_APIROOT");
        }

        if (getenv("YOUTUBE_SCOPE_IGNORE_ACCOUNTS") != nullptr) {
            return;
        }

        /// TODO: The code commented out below should be uncommented as soon as
        /// OnlineAccountClient::refresh_service_statuses() is fixed (Bug #1398813).
        /// For now we have to re-instantiate a new OnlineAccountClient each time.

        ///if (oa_client_ == nullptr) {
            oa_client_.reset(
                    new unity::scopes::OnlineAccountClient(SCOPE_INSTALL_NAME,
                            "sharing", "google"));
        ///} else {
        ///    oa_client_->refresh_service_statuses();
        ///}

        for (auto const& status : oa_client_->get_service_statuses()) {
            if (status.service_authenticated) {
                config_.authenticated = true;
                config_.access_token = status.access_token;
                config_.client_id = status.client_id;
                config_.client_secret = status.client_secret;
                break;
            }
        }

        if (!config_.authenticated) {
            config_.access_token = "";
            config_.client_id = "";
            config_.client_secret = "";
            std::cerr << "YouTube scope is unauthenticated" << std::endl;
        } else {
            std::cerr << "YouTube scope is authenticated" << std::endl;
        }
    }
};

Client::Client(std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client) :
        p(new Priv(oa_client)) {
}

future<SearchListResponse::Ptr> Client::search(const string &query,
        unsigned int max_results, const std::string &category_id) {
    net::Uri::QueryParameters parameters { { "part", "snippet" }, { "type", "video" }, { "q", query } };
    if (max_results > 0)
    {
        parameters.emplace_back(make_pair("maxResults", to_string(max_results)));
    }
    if (!category_id.empty())
    {
        parameters.emplace_back(make_pair("videoCategoryId", category_id));
    }
    return p->async_get<SearchListResponse::Ptr>( { "youtube", "v3", "search" },
            parameters,
            [](const json::Value &root) {
                return make_shared<SearchListResponse>(root);
            });
}

future<Client::GuideCategoryList> Client::guide_categories(
        const string &region_code, const string &locale) {
    return p->async_get<GuideCategoryList>(
            { "youtube", "v3", "guideCategories" }, { { "part", "snippet" }, {
                    "regionCode", region_code }, { "hl", locale } },
            [](const json::Value &root) {
                return get_typed_list<GuideCategory>("youtube#guideCategory", root);
            });
}

future<Client::ChannelList> Client::category_channels(
        const string &categoryId) {
    return p->async_get<ChannelList>( { "youtube", "v3", "channels" }, { {
            "part", "snippet,statistics" }, { "categoryId", categoryId } },
            [](const json::Value &root) {
                return get_typed_list<Channel>("youtube#channel", root);
            });
}

future<Client::ChannelSectionList> Client::channel_sections(
        const string &channelId, int maxResults) {
    return p->async_get<ChannelSectionList>( { "youtube", "v3",
            "channelSections" }, { { "part", "contentDetails" }, { "channelId",
            channelId }, { "maxResults", to_string(maxResults) } },
            [](const json::Value &root) {
                return get_typed_list<ChannelSection>("youtube#channelSection", root);
            });
}

future<Client::VideoList> Client::channel_videos(const string &channelId) {
    return p->async_get<VideoList>( { "youtube", "v3", "search" }, { { "part",
            "snippet" }, { "type", "video" }, { "order", "viewCount" }, {
            "channelId", channelId } }, [](const json::Value &root) {
        return get_typed_list<Video>("youtube#video", root);
    });
}

future<Client::VideoList> Client::chart_videos(const string &chart_name,
        const string &region_code, const std::string &category_id) {
    net::Uri::QueryParameters params = { { "part", "snippet" }, { "regionCode", region_code }, { "chart", chart_name } };

    if (!category_id.empty()) {
        params.emplace_back(make_pair("videoCategoryId", category_id));
    }
    return p->async_get<VideoList>( { "youtube", "v3", "videos" },
            params, [](const json::Value &root) {
                return get_typed_list<Video>("youtube#video", root);
            });
}

future<Client::VideoList> Client::videos(const string &video_id) {
    return p->async_get<VideoList>( { "youtube", "v3", "videos" }, { { "part",
            "snippet,statistics" }, { "id", video_id } },
            [](const json::Value &root) {
                return get_typed_list<Video>("youtube#video", root);
            });
}

future<Client::PlaylistList> Client::channel_playlists(
        const string &channelId) {
    return p->async_get<PlaylistList>( { "youtube", "v3", "playlists" }, { {
            "part", "snippet,contentDetails" }, { "channelId", channelId } },
            [](const json::Value &root) {
                return get_typed_list<Playlist>("youtube#playlist", root);
            });
}

future<Client::PlaylistItemList> Client::playlist_items(
        const string &playlistId) {
    return p->async_get<PlaylistItemList>( { "youtube", "v3", "playlistItems" },
            { { "part", "snippet,contentDetails" }, { "playlistId", playlistId } },
            [](const json::Value &root) {
                return get_typed_list<PlaylistItem>("youtube#playlistItem", root);
            });
}

void Client::cancel() {
    p->cancelled_ = true;
}

bool Client::authenticated() {
    return p->authenticated();
}

void Client::update_config() {
    p->update_config();
}
