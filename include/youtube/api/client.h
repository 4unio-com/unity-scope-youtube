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
#include <youtube/api/channel.h>
#include <youtube/api/channel-section.h>
#include <youtube/api/guide-category.h>
#include <youtube/api/playlist-item.h>
#include <youtube/api/video.h>
#include <youtube/api/video-category.h>

#include <atomic>
#include <deque>
#include <vector>
#include <string>
#include <thread>

#include <core/net/http/client.h>
#include <core/net/http/request.h>

namespace Json {
class Value;
}

namespace youtube {
namespace api {

class Client {
public:
    typedef std::deque<Resource::Ptr> ResourceList;

    typedef std::deque<Channel::Ptr> ChannelList;

    typedef std::deque<ChannelSection::Ptr> ChannelSectionList;

    typedef std::deque<GuideCategory::Ptr> GuideCategoryList;

    typedef std::deque<VideoCategory::Ptr> VideoCategoryList;

    typedef std::deque<PlaylistItem::Ptr> PlaylistItemList;

    typedef std::deque<Video::Ptr> VideoList;

    Client(Config::Ptr config, int cardinality, const std::string& locale);

    virtual ~Client();

    virtual VideoCategoryList video_categories();

    virtual GuideCategoryList guide_categories();

    virtual ResourceList search(const std::string &query = std::string());

    virtual ChannelList category_channels(const std::string &categoryId);

    virtual ChannelSectionList channel_sections(const std::string &channelId,
            int maxResults);

    virtual VideoList channel_videos(const std::string &channelId);

    virtual PlaylistItemList playlist_items(const std::string &playlistId);

    virtual ResourceList feed();

    virtual void cancel();

    virtual Config::Ptr config();

protected:
    void get(const std::deque<std::string> &endpoint,
            const std::vector<std::pair<std::string, std::string>> &querys,
            Json::Value &root);

    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

    std::shared_ptr<core::net::http::Client> client_;

    std::thread worker_;

    Config::Ptr config_;

    int cardinality_;

    std::string locale_;

    std::atomic<bool> cancelled_;
};

}
}

#endif // YOUTUBE_API_CLIENT_H_
