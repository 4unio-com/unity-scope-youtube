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

#ifndef YOUTUBE_API_CLIENT_H_
#define YOUTUBE_API_CLIENT_H_

#include <youtube/api/config.h>
#include <youtube/api/video.h>
#include <youtube/api/video-category.h>

#include <atomic>
#include <deque>
#include <vector>
#include <string>
#include <core/net/http/request.h>

namespace Json {
class Value;
}

namespace youtube {
namespace api {

class Client {
public:
    typedef std::deque<Resource::Ptr> ResourceList;

    typedef std::deque<VideoCategory::Ptr> VideoCategoryList;

    typedef std::deque<Video::Ptr> VideoList;

    Client(Config::Ptr config);

    virtual ~Client() = default;

    virtual VideoCategoryList video_categories();

    virtual ResourceList search(const std::string &query = std::string());

    virtual ResourceList category_videos(const std::string &category);

    virtual ResourceList channel_videos(const std::string &channel);

    virtual ResourceList playlist_videos(const std::string &channel);

    virtual ResourceList feed();

    virtual void cancel();

    virtual Config::Ptr config();

protected:
    void get(const std::deque<std::string> &endpoint,
            const std::vector<std::pair<std::string, std::string>> &querys,
            Json::Value &root);

    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

    Config::Ptr config_;

    std::atomic<bool> cancelled_;
};

}
}

#endif // YOUTUBE_API_CLIENT_H_
