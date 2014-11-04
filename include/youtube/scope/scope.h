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

#ifndef YOUTUBE_SCOPE_SCOPE_H_
#define YOUTUBE_SCOPE_SCOPE_H_

#include <youtube/api/config.h>

#include <unity/scopes/OnlineAccountClient.h>
#include <unity/scopes/PreviewQueryBase.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/ScopeBase.h>

#include <condition_variable>

namespace youtube {

namespace scope {

class Scope: public unity::scopes::ScopeBase {
public:
    void start(std::string const&) override;

    void stop() override;

    unity::scopes::PreviewQueryBase::UPtr preview(const unity::scopes::Result&,
            const unity::scopes::ActionMetadata&) override;

    virtual unity::scopes::SearchQueryBase::UPtr search(
            unity::scopes::CannedQuery const& q,
            unity::scopes::SearchMetadata const&) override;

protected:
    void service_update(unity::scopes::OnlineAccountClient::ServiceStatus const& status);

    void update_config();

    void init_config();

    youtube::api::Config::Ptr config_;
    std::mutex config_mutex_;
    std::condition_variable config_cond_;
    std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client_;
};

}
}

#endif // YOUTUBE_SCOPE_SCOPE_H_
