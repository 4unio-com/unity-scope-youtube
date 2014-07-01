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

#include <boost/algorithm/string/trim.hpp>

#include <youtube/api/channel.h>
#include <youtube/api/playlist.h>

#include <youtube/scope/query.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchMetadata.h>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace youtube::api;
using namespace youtube::scope;

const static string SEARCH_CATEGORY_TEMPLATE =
        R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "small",
    "overlay": false,
    "collapsed-rows": 1
  },
  "components": {
    "title": "title",
    "art" : {
      "field": "art",
      "aspect-ratio": 1.5
    },
    "subtitle": "username"
  }
}
)";

const static string SEARCH_VIDEO_TEMPLATE =
        R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "overlay": true
  },
  "components": {
    "title": "title",
    "art" : {
      "field": "art",
      "aspect-ratio": 2.0
    },
    "subtitle": "username"
  }
}
)";

const static string SEARCH_CATEGORY_LOGIN_NAG =
        R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "card-background": "color:///#DD4814"
  },
  "components": {
    "title": "title",
    "background": "background",
    "art" : {
      "aspect-ratio": 100.0
    }
  }
}
)";

Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
        Config::Ptr config) :
        sc::SearchQueryBase(query, metadata), client_(config,
                metadata.cardinality(), metadata.locale()) {
}

Query::~Query() {
}

void Query::cancelled() {
    client_.cancel();
}

void Query::add_login_nag(const sc::SearchReplyProxy &reply) {
    sc::CategoryRenderer rdr(SEARCH_CATEGORY_LOGIN_NAG);
    auto cat = reply->register_category("youtube_login_nag", "", "", rdr);

    sc::CategorisedResult res(cat);
    res.set_title("Log-in to Youtube");
    res.set_uri("settings:///system/online-accounts");
    reply->push(res);
}

void push_playlist_item(const sc::SearchReplyProxy &reply,
        const sc::Category::SCPtr &category, const PlaylistItem::Ptr &item) {
    sc::CategorisedResult res(category);
    res.set_title(item->title());
    res.set_art(item->picture());
    res["link"] = item->link();
    res["description"] = item->description();
    res["username"] = item->username();
    res.set_uri(item->id());

    cerr << "    item: " << item->id() << " " << item->title() << endl;

    if (!reply->push(res)) {
        return;
    }
}

void push_video(const sc::SearchReplyProxy &reply,
        const sc::Category::SCPtr &category, const Video::Ptr &video) {
    sc::CategorisedResult res(category);
    res.set_title(video->title());
    res.set_art(video->picture());
    res["link"] = video->link();
    res["description"] = video->description();
    res["username"] = video->username();
    res.set_uri(video->id());

    cerr << "    video: " << video->id() << " " << video->title() << endl;

    if (!reply->push(res)) {
        return;
    }
}

void Query::run(sc::SearchReplyProxy const& reply) {
    try {
        const sc::CannedQuery &query(sc::SearchQueryBase::query());
        string query_string = alg::trim_copy(query.query_string());

        auto popular = reply->register_category("youtube-popular", "", "",
                sc::CategoryRenderer(SEARCH_VIDEO_TEMPLATE));
//        Client::ResourceList resources;

        if (query_string.empty()) {
            sc::Department::SPtr all_depts = sc::Department::create("", query,
                    "My Feed");
            cerr << "Categories:" << endl;
            for (GuideCategory::Ptr category : client_.guide_categories()) {
                sc::Department::SPtr dept = sc::Department::create(
                        "guideCategory:" + category->id(), query,
                        category->title());

                cerr << "  guideCategory: " << category->id() << " "
                        << category->title() << endl;
                all_depts->add_subdepartment(dept);
            }
            reply->register_departments(all_depts);

            string department_id = query.department_id();
            if (!department_id.empty()) {

                if (department_id.find("videoCategory:") == 0) {
//                    resources = client_.category_channels(
//                            department_id.substr(14));
                } else if (department_id.find("guideCategory:") == 0) {
                    bool first = true;
                    cerr << "Finding channels" << endl;
                    for (Channel::Ptr channel : client_.category_channels(
                            department_id.substr(14))) {
                        cerr << "  channel: " << channel->id() << " "
                                << channel->title() << endl;
                        Client::ChannelSectionList sections =
                                client_.channel_sections(channel->id(), 1);

                        ChannelSection::Ptr section;
                        for (auto it : sections) {
                            if (!it->playlist_id().empty()) {
                                section = it;
                                break;
                            }
                        }

                        cerr << "  section: " << section->id() << " "
                                << section->playlist_id() << endl;

                        if (!section) {
                            cerr << "    empty playlist" << endl;
                            continue;
                        }

                        Client::PlaylistItemList items = client_.playlist_items(
                                section->playlist_id());

//                        Client::VideoList videos = client_.channel_videos(
//                                channel->id());
                        auto it = items.cbegin();

                        if (first) {
                            first = false;
                            if (it != items.cend()) {
                                PlaylistItem::Ptr video(*it);
                                push_playlist_item(reply, popular, video);
                                ++it;
                            }
                        }

                        auto cat = reply->register_category(channel->id(),
                                channel->title(), "",
                                sc::CategoryRenderer(SEARCH_CATEGORY_TEMPLATE));
                        for (; it != items.cend(); ++it) {
                            PlaylistItem::Ptr video(*it);
                            push_playlist_item(reply, cat, video);
                        }
                    }
                } else if (department_id.find("channel:") == 0) {
//                    sc::Department::SPtr dummy_dept = sc::Department::create(
//                            department_id, query, "Channel Department");
//                    all_depts->add_subdepartment(dummy_dept);
//                    resources = client_.channel_videos(department_id.substr(8));
                } else if (department_id.find("playlist:") == 0) {
//                    sc::Department::SPtr dummy_dept = sc::Department::create(
//                            department_id, query, "Channel Playlist");
//                    all_depts->add_subdepartment(dummy_dept);
//                    resources = client_.playlist_videos(
//                            department_id.substr(9));
                }
            } else {
//                resources = client_.feed();
            }

        } else {
//            resources = client_.search(query_string);
        }

//
//        for (const Resource::Ptr& resource : resources) {
//            sc::CategorisedResult res(cat);
//            res.set_title(resource->title());
//            res.set_art(resource->picture());
//
//            sc::CannedQuery new_query("unity-scope-youtube");
//
//            switch (resource->kind()) {
//            case Resource::Kind::channel: {
//                Channel::Ptr channel(static_pointer_cast<Channel>(resource));
//
//                sc::FilterState filter_state;
//                new_query.set_department_id("channel:" + channel->id());
//                res.set_uri(new_query.to_uri());
//
//                break;
//            }
//            case Resource::Kind::guideCategory: {
//                GuideCategory::Ptr channel(
//                        static_pointer_cast<GuideCategory>(resource));
//
//                new_query.set_department_id("guideCategory:" + channel->id());
//                res.set_uri(new_query.to_uri());
//                break;
//            }
//            case Resource::Kind::playlist: {
//                Playlist::Ptr playlist(static_pointer_cast<Playlist>(resource));
//                new_query.set_department_id("playlist:" + playlist->id());
//                res.set_uri(new_query.to_uri());
//                break;
//            }
//            case Resource::Kind::video: {
//                Video::Ptr video(static_pointer_cast<Video>(resource));
//                res["link"] = video->link();
//                res["description"] = video->description();
//                res["username"] = video->username();
//                res.set_uri(video->id());
//                break;
//            }
//            case Resource::Kind::videoCategory: {
//                VideoCategory::Ptr videoCategory(
//                        static_pointer_cast<VideoCategory>(resource));
//                new_query.set_department_id(
//                        "videoCategory:" + videoCategory->id());
//                res.set_uri(new_query.to_uri());
//                break;
//            }
//            }
//
//            if (!reply->push(res)) {
//                return;
//            }
//        }

//        FIXME Add this back when direct activation can be controlled
//        if (!client_.config()->authenticated) {
//            add_login_nag(reply);
//        }
    } catch (domain_error &e) {
    }
}

