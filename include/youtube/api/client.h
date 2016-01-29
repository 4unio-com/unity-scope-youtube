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

#ifndef YOUTUBE_API_CLIENT_H_
#define YOUTUBE_API_CLIENT_H_

#include <youtube/api/config.h>
#include <youtube/api/channel.h>
#include <youtube/api/subscription.h>
#include <youtube/api/subscription-item.h>
#include <youtube/api/channel-section.h>
#include <youtube/api/guide-category.h>
#include <youtube/api/playlist.h>
#include <youtube/api/playlist-item.h>
#include <youtube/api/search-list-response.h>
#include <youtube/api/video.h>
#include <youtube/api/comment.h>

#include <unity/scopes/OnlineAccountClient.h>

#include <atomic>
#include <deque>
#include <future>
#include <string>
#include <thread>
#include <vector>

namespace youtube {
namespace api {

class Client {
public:
    typedef std::shared_ptr<Client> Ptr;

    typedef std::deque<Channel::Ptr> ChannelList;

    typedef std::deque<Subscription::Ptr> SubscriptionList;

    typedef std::deque<SubscriptionItem::Ptr> SubscriptionItemList;

    typedef std::deque<ChannelSection::Ptr> ChannelSectionList;

    typedef std::deque<GuideCategory::Ptr> GuideCategoryList;

    typedef std::deque<PlaylistItem::Ptr> PlaylistItemList;

    typedef std::deque<Playlist::Ptr> PlaylistList;

    typedef std::deque<Video::Ptr> VideoList;
    
    typedef std::deque<Comment::Ptr> CommentList;

    Client(std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client);

    virtual ~Client() = default;

    virtual std::future<GuideCategoryList> guide_categories(
            const std::string &region_code, const std::string &locale);

    virtual std::future<SearchListResponse::Ptr> search(
            const std::string &query, unsigned int max_results, const std::string &category_id="");

    virtual std::future<SubscriptionList> subscription_channels();

    virtual std::future<ChannelList> auth_user_info();

    virtual std::future<std::string> subscription_channel_uploads(std::string const &department_id);

    virtual std::future<SubscriptionItemList> subscription_items( const std::string &playlistId);

    virtual std::future<ChannelList> category_channels(
            const std::string &categoryId);

    virtual std::future<ChannelList> channels_statistics(
            const std::string &channelId);

    virtual std::future<ChannelSectionList> channel_sections(
            const std::string &channelId, int maxResults);

    virtual std::future<VideoList> channel_videos(const std::string &channelId);

    virtual std::future<VideoList> chart_videos(const std::string &chart_name,
            const std::string &region_code, const std::string &category_id);

    virtual std::future<PlaylistList> channel_playlists(
            const std::string &channelId);

    virtual std::future<PlaylistItemList> playlist_items(
            const std::string &playlistId);

    virtual std::future<VideoList> videos(const std::string &videoId);

    virtual std::future<CommentList> video_comments(const std::string &videoId);
    
    virtual std::future<bool> post_comments(const std::string &videoId, const std::string msg);

    virtual std::future<bool> rate(const std::string &videoId, bool likes);

    virtual std::future<Client::SubscriptionList> subscribeId(const std::string &channelId);

    virtual std::future<bool> subscribe(const std::string &channelId);
    
    virtual std::future<bool> unSubscribe(const std::string &subscribeId);
    
    virtual std::future<bool> addVideoIntoPlayList(const std::string &videoId,
                                                   const std::string &playlistId);

    virtual void cancel();

    virtual bool authenticated();

protected:
    class Priv;
    friend Priv;

    std::shared_ptr<Priv> p;
};

}
}

#endif // YOUTUBE_API_CLIENT_H_
