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

#ifndef YOUTUBE_SCOPE_QUERY_H_
#define YOUTUBE_SCOPE_QUERY_H_

#include <youtube/api/client.h>

#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>

namespace youtube {
namespace scope {

class Query: public unity::scopes::SearchQueryBase {
public:
    Query(const unity::scopes::CannedQuery &query,
            const unity::scopes::SearchMetadata &metadata,
            youtube::api::Client::Ptr client);

    ~Query() = default;

    void cancelled() override;

    void run(const unity::scopes::SearchReplyProxy &reply) override;

protected:
    youtube::api::Client::GuideCategoryList load_departments(
            const unity::scopes::SearchReplyProxy &reply);

    void add_login_nag(const unity::scopes::SearchReplyProxy &reply);

    void guide_category(const unity::scopes::SearchReplyProxy &reply,
            const std::string &department_id);

    void guide_category_videos(const unity::scopes::SearchReplyProxy &reply,
            const std::string &department_id);

    void guide_category_channels(const unity::scopes::SearchReplyProxy &reply,
            const std::string &department_id);

    void guide_category_playlists(const unity::scopes::SearchReplyProxy &reply,
            const std::string &department_id);

    void playlist(const unity::scopes::SearchReplyProxy &reply,
            const std::string &playlist_id);

    void surfacing(const unity::scopes::SearchReplyProxy &reply);

    void search(const unity::scopes::SearchReplyProxy &reply,
            const std::string &query_string);

    youtube::api::Client::Ptr client_;
};

}
}

#endif // YOUTUBE_SCOPE_QUERY_H_

