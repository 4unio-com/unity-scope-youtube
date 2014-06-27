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

#include <youtube/scope/query.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace youtube::api;
using namespace youtube::scope;

const static string SEARCH_CATEGORY_TEMPLATE = R"(
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

const static string SEARCH_CATEGORY_LOGIN_NAG = R"(
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
        sc::SearchQueryBase(query, metadata), client_(config) {
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

void Query::run(sc::SearchReplyProxy const& reply) {
    try {
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        string query_string = alg::trim_copy(query.query_string());

        Client::VideoList videos;

        if (query_string.empty()) {

            sc::Department::SPtr all_depts = sc::Department::create("", query,
                    "My Feed");
            for (VideoCategory::Ptr channel : client_.video_categories()) {
                sc::Department::SPtr dept = sc::Department::create(
                        channel->id(), query, channel->name());
                all_depts->add_subdepartment(dept);
            }
            reply->register_departments(all_depts);

            if (!query.department_id().empty()) {
                videos = client_.category_videos(query.department_id());
            } else {
                videos = client_.feed();
            }
        } else {
            videos = client_.videos(query_string);
        }

        auto cat = reply->register_category("youtube", "Youtube", "",
                sc::CategoryRenderer(SEARCH_CATEGORY_TEMPLATE));

        for (Video::Ptr video : videos) {
            sc::CategorisedResult res(cat);
            res.set_uri(video->uri());
            res.set_title(video->name());
            res.set_art(video->picture());
            res["description"] = video->description();
            res["username"] = video->username();

            if (!reply->push(res)) {
                return;
            }
        }

//        FIXME Add this back when direct activation can be controlled
//        if (!client_.config()->authenticated) {
//            add_login_nag(reply);
//        }
    } catch (domain_error &e) {
    }
}

