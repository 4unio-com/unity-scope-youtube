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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <youtube/api/channel.h>
#include <youtube/api/playlist.h>

#include <youtube/scope/query.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/OnlineAccountClient.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchMetadata.h>

#include <sstream>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace youtube::api;
using namespace youtube::scope;

namespace {
const static string BROWSE_TEMPLATE =
        R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "medium",
    "overlay": false,
    "collapsed-rows": 1
  },
  "components": {
    "title": "title",
    "art" : {
      "field": "art",
      "aspect-ratio": 1.5
    },
    "subtitle": "subtitle"
  }
}
)";

const static string SEARCH_TEMPLATE =
        R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "medium",
    "card-layout": "horizontal"
  },
  "components": {
    "title": "title",
    "art" : {
      "field": "art",
      "aspect-ratio": 1.7
    },
    "subtitle": "subtitle"
  }
}
)";

const static string POPULAR_TEMPLATE =
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
    "subtitle": "subtitle"
  }
}
)";

const static string SEARCH_CATEGORY_LOGIN_NAG =
        R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-total_results": "large",
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

template<typename T>
static T get_or_throw(future<T> &f) {
    if (f.wait_for(std::chrono::seconds(10)) != future_status::ready) {
        throw domain_error("HTTP request timeout");
    }
    return f.get();
}

enum class DepartmentType {
    guide_category, channel, playlist, aggregated
};

enum class SectionType {
    none, videos, playlists, channels
};

/**
 * This class helps to encode/decode the department identity to/from string form
 */
struct DepartmentPath {
    DepartmentType department_type;
    string department;
    SectionType section_type = SectionType::none;

    DepartmentPath(DepartmentType department_type_, const string &department_,
            SectionType section_type_ = SectionType::none) :
            department_type(department_type_), department(department_), section_type(
                    section_type_) {
    }

    DepartmentPath(const string &s) {
        if (alg::starts_with(s, "guideCategory:")) {
            department_type = DepartmentType::guide_category;
        } else if (alg::starts_with(s,"guideCategory-videos:")) {
            department_type = DepartmentType::guide_category;
            section_type = SectionType::videos;
        } else if (alg::starts_with(s, "guideCategory-playlists:")) {
            department_type = DepartmentType::guide_category;
            section_type = SectionType::playlists;
        } else if (alg::starts_with(s, "guideCategory-channels:")) {
            department_type = DepartmentType::guide_category;
            section_type = SectionType::channels;
        } else if (alg::starts_with(s, "channel:")) {
            department_type = DepartmentType::channel;
        } else if (alg::starts_with(s, "playlist:")) {
            department_type = DepartmentType::playlist;
        } else if (alg::starts_with(s, "aggregated:")) {
            department_type = DepartmentType::aggregated;
        }

        department = s.substr(s.find(':') + 1);
    }

    string to_string() const {
        ostringstream result;
        switch (department_type) {
        case DepartmentType::guide_category: {
            switch (section_type) {
            case SectionType::none:
                result << "guideCategory:";
                break;
            case SectionType::videos:
                result << "guideCategory-videos:";
                break;
            case SectionType::playlists:
                result << "guideCategory-playlists:";
                break;
            case SectionType::channels:
                result << "guideCategory-channels:";
                break;
            }
            break;
        }
        case DepartmentType::playlist:
            result << "playlist:";
            break;
        case DepartmentType::channel:
            result << "channel:";
            break;
        case DepartmentType::aggregated:
            result << "aggregated:";
            break;
        }

        result << department;
        return result.str();
    }
};

void push_resource(const sc::SearchReplyProxy &reply,
        const sc::Category::SCPtr &category, const Resource::Ptr &resource) {
    sc::CategorisedResult res(category);
    res.set_title(resource->title());
    res.set_art(resource->picture());
    res["kind"] = resource->kind_str();

    sc::CannedQuery new_query(SCOPE_INSTALL_NAME);

    switch (resource->kind()) {
    case Resource::Kind::channel: {
        Channel::Ptr channel(static_pointer_cast<Channel>(resource));
        DepartmentPath path { DepartmentType::channel, channel->id() };
        new_query.set_department_id(path.to_string());
        res.set_uri(new_query.to_uri());
        res["subtitle"] = channel->subscriber_count() + " subscribers";
        res["description"] = channel->description();
        break;
    }
    case Resource::Kind::channelSection: {
        break;
    }
    case Resource::Kind::guideCategory: {
        GuideCategory::Ptr guide_category(
                static_pointer_cast<GuideCategory>(resource));
        DepartmentPath path { DepartmentType::guide_category,
                guide_category->id() };
        new_query.set_department_id(path.to_string());
        res.set_uri(new_query.to_uri());
        break;
    }
    case Resource::Kind::playlist: {
        Playlist::Ptr playlist(static_pointer_cast<Playlist>(resource));
        DepartmentPath path { DepartmentType::playlist, playlist->id() };
        new_query.set_department_id(path.to_string());
        res.set_uri(new_query.to_uri());
        res["subtitle"] = to_string(playlist->item_count()) + " videos";
        res["description"] = playlist->description();
        break;
    }
    case Resource::Kind::playlistItem: {
        PlaylistItem::Ptr playlist_item(
                static_pointer_cast<PlaylistItem>(resource));
        res["link"] = playlist_item->link();
        res["description"] = playlist_item->description();
        res["subtitle"] = playlist_item->username();
        res.set_uri(playlist_item->video_id());
        break;
    }
    case Resource::Kind::video: {
        Video::Ptr video(static_pointer_cast<Video>(resource));
        res["link"] = video->link();
        res["description"] = video->description();
        res["subtitle"] = video->username();
        res.set_uri(video->id());
        break;
    }
    }

    if (!reply->push(res)) {
        return;
    }
}

}

Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
        Client::Ptr client) :
        sc::SearchQueryBase(query, metadata), client_(client) {
}

void Query::cancelled() {
    client_->cancel();
}

void Query::add_login_nag(const sc::SearchReplyProxy &reply) {
    sc::CategoryRenderer rdr(SEARCH_CATEGORY_LOGIN_NAG);
    auto cat = reply->register_category("youtube_login_nag", "", "", rdr);

    sc::CategorisedResult res(cat);
    res.set_title("Log-in to YouTube");

    sc::OnlineAccountClient oa_client("com.ubuntu.scopes.youtube_youtube", "sharing", "google");
    oa_client.register_account_login_item(res,
                                          query(),
                                          sc::OnlineAccountClient::InvalidateResults,
                                          sc::OnlineAccountClient::DoNothing);

    reply->push(res);
}

void Query::guide_category(const sc::SearchReplyProxy &reply,
        const string &department_id) {
    auto popular = reply->register_category("youtube-popular", "", "",
            sc::CategoryRenderer(POPULAR_TEMPLATE));

    bool first = true;
    cerr << "Finding channels: " << department_id << endl;

    auto channels_future = client_->category_channels(department_id);
    auto channels = get_or_throw(channels_future);
    deque<future<Client::ChannelSectionList>> channel_section_futures;
    for (Channel::Ptr channel : channels) {
        channel_section_futures.emplace_back(
                client_->channel_sections(channel->id(), 1));
        cerr << "  channel: " << channel->id() << " " << channel->title()
                << endl;
    }

    int channel_number = 0;
    for (future<Client::ChannelSectionList> &channel_section_future : channel_section_futures) {
        Channel::Ptr channel = channels.at(channel_number++);

        Client::ChannelSectionList sections = get_or_throw(
                channel_section_future);

        ChannelSection::Ptr section;
        for (auto it : sections) {
            if (!it->playlist_id().empty()) {
                section = it;
                break;
            }
        }

        if (!section) {
            cerr << "    empty playlist" << endl;
            continue;
        }

        cerr << "  section: " << section->id() << " " << section->playlist_id()
                << endl;

        auto playlist_future = client_->playlist_items(section->playlist_id());
        Client::PlaylistItemList items = get_or_throw(playlist_future);

        auto it = items.cbegin();

        if (first) {
            first = false;
            if (it != items.cend()) {
                PlaylistItem::Ptr video(*it);
                push_resource(reply, popular, video);
                ++it;
            }
        }

        auto cat = reply->register_category(channel->id(), channel->title(), "",
                sc::CategoryRenderer(BROWSE_TEMPLATE));
        for (; it != items.cend(); ++it) {
            PlaylistItem::Ptr video(*it);
            push_resource(reply, cat, video);
        }
    }
}

void Query::guide_category_videos(const sc::SearchReplyProxy &reply,
        const string &department_id) {
    cerr << "Finding videos: " << department_id << endl;

    auto cat = reply->register_category("youtube", "Videos", "",
            sc::CategoryRenderer(SEARCH_TEMPLATE));

    auto channels_future = client_->category_channels(department_id);
    auto channels = get_or_throw(channels_future);
    deque<future<Client::VideoList>> videos_futures;
    for (Channel::Ptr channel : channels) {
        cerr << "  channel: " << channel->id() << " " << channel->title()
                << endl;
        videos_futures.emplace_back(client_->channel_videos(channel->id()));
    }

    for (auto &it : videos_futures) {
        Client::VideoList videos = it.get();
        for (auto &video : videos) {
            cerr << "    video: " << video->id() << " " << video->title()
                    << endl;
            push_resource(reply, cat, video);
        }
    }
}

void Query::guide_category_channels(const sc::SearchReplyProxy &reply,
        const string &department_id) {
    cerr << "Finding channels: " << department_id << endl;

    auto cat = reply->register_category("youtube", "Channels", "",
            sc::CategoryRenderer(SEARCH_TEMPLATE));
    auto channels_future = client_->category_channels(department_id);
    auto channels = get_or_throw(channels_future);
    for (Channel::Ptr channel : channels) {
        push_resource(reply, cat, channel);
        cerr << "  channel: " << channel->id() << " " << channel->title()
                << endl;
    }
}

void Query::guide_category_playlists(const sc::SearchReplyProxy &reply,
        const string &department_id) {
    cerr << "Finding playlists: " << department_id << endl;

    auto cat = reply->register_category("youtube", "Playlists", "",
            sc::CategoryRenderer(SEARCH_TEMPLATE));

    auto channels_future = client_->category_channels(department_id);
    auto channels = get_or_throw(channels_future);
    deque<future<Client::PlaylistList>> playlists_futures;
    for (Channel::Ptr channel : channels) {
        cerr << "  channel: " << channel->id() << " " << channel->title()
                << endl;
        playlists_futures.emplace_back(
                client_->channel_playlists(channel->id()));
    }

    for (auto &it : playlists_futures) {
        Client::PlaylistList playlists = it.get();
        for (auto &playlist : playlists) {
            cerr << "    playlist: " << playlist->id() << " "
                    << playlist->title() << endl;
            push_resource(reply, cat, playlist);
        }
    }
}

void Query::playlist(const sc::SearchReplyProxy &reply,
        const string &playlist_id) {
    cerr << "Playlist: " << playlist_id << endl;

    auto cat = reply->register_category("youtube", "Playlist contents", "",
            sc::CategoryRenderer(SEARCH_TEMPLATE));

    auto playlist_future = client_->playlist_items(playlist_id);
    Client::PlaylistItemList items = get_or_throw(playlist_future);

    for (auto &playlist : items) {
        push_resource(reply, cat, playlist);
    }
}

void Query::channel(const sc::SearchReplyProxy &reply,
        const string &channel_id) {
    cerr << "Channel: " << channel_id << endl;

    auto cat = reply->register_category("youtube", "Channel contents", "",
            sc::CategoryRenderer(SEARCH_TEMPLATE));

    auto channels_future = client_->channel_videos(channel_id);
    Client::VideoList videos = get_or_throw(channels_future);

    for (auto &video : videos) {
        push_resource(reply, cat, video);
    }
}

void Query::popular_videos(const sc::SearchReplyProxy &reply) {
    auto resources_future = client_->chart_videos("mostPopular", country_code());
    auto resources = get_or_throw(resources_future);

    auto cat = reply->register_category("youtube", "YouTube", "",
                                        sc::CategoryRenderer(SEARCH_TEMPLATE));
    for (const Resource::Ptr& resource : resources) {
        push_resource(reply, cat, resource);
    }
}

string Query::country_code() const {
    string country_code = "US";
    auto metadata = search_metadata();
    if (metadata.has_location()) {
        auto location = metadata.location();
        if (location.has_country_code()) {
            country_code = location.country_code();
        }
    }
    return country_code;
}

void Query::surfacing(const sc::SearchReplyProxy &reply) {
    const sc::CannedQuery &query(sc::SearchQueryBase::query());

    sc::Department::SPtr all_depts;
    bool first_dept = true;

    auto departments_future = client_->guide_categories(country_code(),
            search_metadata().locale());
    auto departments = get_or_throw(departments_future);
    for (GuideCategory::Ptr category : departments) {
        if (first_dept) {
            first_dept = false;
            all_depts = sc::Department::create("", query, category->title());
        } else {
            DepartmentPath path { DepartmentType::guide_category,
                    category->id(), SectionType::none };
            sc::Department::SPtr dept = sc::Department::create(path.to_string(),
                    query, category->title());
            all_depts->add_subdepartment(dept);

            DepartmentPath videos_path { DepartmentType::guide_category,
                    category->id(), SectionType::videos };
            sc::Department::SPtr videos = sc::Department::create(
                    videos_path.to_string(), query, "Videos");
            dept->add_subdepartment(videos);

            DepartmentPath playlists_path { DepartmentType::guide_category,
                    category->id(), SectionType::playlists };
            sc::Department::SPtr playlists = sc::Department::create(
                    playlists_path.to_string(), query, "Playlists");
            dept->add_subdepartment(playlists);

            DepartmentPath channels_path { DepartmentType::guide_category,
                    category->id(), SectionType::channels };
            sc::Department::SPtr channels = sc::Department::create(
                    channels_path.to_string(), query, "Channels");
            dept->add_subdepartment(channels);
        }
    }


    string raw_department_id = query.department_id();
    if (!raw_department_id.empty()) {
        DepartmentPath path(raw_department_id);
        switch (path.department_type) {
        case DepartmentType::guide_category: {
            // FIXME Working around the UI bug (have to register departments before results)
            reply->register_departments(all_depts);

            switch (path.section_type) {
            case SectionType::none: {
                // If we have picked a top level department
                guide_category(reply, path.department);
                break;
            }
            case SectionType::videos: {
                // If we only want to see the videos of a department
                guide_category_videos(reply, path.department);
                break;
            }
            case SectionType::channels: {
                // If we only want to see the channels of a department
                guide_category_channels(reply, path.department);
                break;
            }
            case SectionType::playlists: {
                // If we only want to see the playlists of a department
                guide_category_playlists(reply, path.department);
                break;
            }
            }
            break;
        }
        case DepartmentType::playlist: {
            // If we click on a playlist in the search results

            // Need to add a dummy department to pass the validation check
            sc::Department::SPtr dummy = sc::Department::create(
                    raw_department_id, query, " ");
            all_depts->add_subdepartment(dummy);
            reply->register_departments(all_depts);

            playlist(reply, path.department);
            break;
        }
        case DepartmentType::channel: {
            // If we click on a channel in the search results

            // Need to add a dummy department to pass the validation check
            sc::Department::SPtr dummy = sc::Department::create(
                    raw_department_id, query, " ");
            all_depts->add_subdepartment(dummy);
            reply->register_departments(all_depts);

            channel(reply, path.department);

            break;
        }
        case DepartmentType::aggregated: {
            // If another scope has asked us to surface results

            // Need to add a dummy department to pass the validation check
            sc::Department::SPtr dummy = sc::Department::create(
                    raw_department_id, query, " ");
            all_depts->add_subdepartment(dummy);
            reply->register_departments(all_depts);

            popular_videos(reply);

            break;
        }
        }
    } else {
        // This is the initial surfacing screen

        // FIXME Working around the UI bug (have to register departments before results)
        reply->register_departments(all_depts);

        guide_category(reply, departments.at(0)->id());
    }
}

void Query::search(const sc::SearchReplyProxy &reply,
        const string &query_string) {
    auto resources_future = client_->search(query_string, search_metadata().cardinality());
    auto resources = get_or_throw(resources_future);

    auto cat = reply->register_category("youtube",
            to_string(resources->total_results()) + " results from YouTube", "",
            sc::CategoryRenderer(SEARCH_TEMPLATE));
    for (const Resource::Ptr& resource : resources->items()) {
        push_resource(reply, cat, resource);
    }
}

void Query::run(sc::SearchReplyProxy const& reply) {
    try {
        const sc::CannedQuery &query(sc::SearchQueryBase::query());
        string query_string = alg::trim_copy(query.query_string());

        if (query_string.empty()) {
            surfacing(reply);
        } else {
            search(reply, query_string);
        }

        if (!client_->config()->authenticated) {
            add_login_nag(reply);
        }
    } catch (domain_error &e) {
        cerr << "ERROR: " << e.what() << endl;
    }
}

