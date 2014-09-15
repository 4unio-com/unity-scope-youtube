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

#include <youtube/scope/localisation.h>
#include <youtube/scope/scope.h>
#include <youtube/scope/query.h>
#include <youtube/scope/preview.h>

#include <iostream>
#include <sstream>
#include <fstream>

namespace sc = unity::scopes;
using namespace std;
using namespace youtube::scope;
using namespace youtube::api;

void Scope::service_update(sc::OnlineAccountClient::ServiceStatus const&)
{
    update_config();
}

void Scope::update_config()
{
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_ = make_shared<Config>();

    if (getenv("YOUTUBE_SCOPE_APIROOT")) {
        config_->apiroot = getenv("YOUTUBE_SCOPE_APIROOT");
    }

    if (getenv("YOUTUBE_SCOPE_IGNORE_ACCOUNTS") == nullptr) {
        for (auto const& status : oa_client_->get_service_statuses())
        {
            if (status.service_authenticated)
            {
                config_->authenticated = true;
                config_->access_token = status.access_token;
                config_->client_id = status.client_id;
                config_->client_secret = status.client_secret;
                break;
            }
        }
    }

    if (!config_->authenticated) {
        cerr << "YouTube scope is unauthenticated" << endl;
    } else {
        cerr << "YouTube scope is authenticated" << endl;
    }
}

void Scope::start(string const&) {
    setlocale(LC_ALL, "");
    string translation_directory = ScopeBase::scope_directory()
            + "/../share/locale/";
    bindtextdomain(GETTEXT_PACKAGE, translation_directory.c_str());

    oa_client_.reset(
            new sc::OnlineAccountClient(SCOPE_INSTALL_NAME,
                    "sharing", "google",
                    sc::OnlineAccountClient::CreateInternalMainLoop));
    oa_client_->set_service_update_callback(
            std::bind(&Scope::service_update, this, std::placeholders::_1));

    update_config();
}

void Scope::stop() {
}

sc::SearchQueryBase::UPtr Scope::search(const sc::CannedQuery &query,
        const sc::SearchMetadata &metadata) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto client = make_shared<Client>(config_);
    return sc::SearchQueryBase::UPtr(new Query(query, metadata, client));
}

sc::PreviewQueryBase::UPtr Scope::preview(sc::Result const& result,
        sc::ActionMetadata const& metadata) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto client = make_shared<Client>(config_);
    return sc::PreviewQueryBase::UPtr(new Preview(result, metadata, client));
}

#define EXPORT __attribute__ ((visibility ("default")))

extern "C" {

EXPORT
unity::scopes::ScopeBase*
// cppcheck-suppress unusedFunction
UNITY_SCOPE_CREATE_FUNCTION() {
    return new Scope();
}

EXPORT
void
// cppcheck-suppress unusedFunction
UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base) {
    delete scope_base;
}

}
