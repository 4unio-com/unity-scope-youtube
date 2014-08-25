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

void Scope::start(string const&) {
    config_ = make_shared<Config>();

    if (getenv("YOUTUBE_SCOPE_APIROOT")) {
        config_->apiroot = getenv("YOUTUBE_SCOPE_APIROOT");
    }

    SimpleOAuth oauth("com.ubuntu.scopes.youtube_youtube");
    SimpleOAuth::AuthData auth_data;
    if (getenv("YOUTUBE_SCOPE_IGNORE_ACCOUNTS") == nullptr) {
        auth_data = oauth.auth_data();
    }
    if (auth_data.access_token.empty()) {
        cerr << "YouTube scope is unauthenticated" << endl;
    } else {
        cerr << "YouTube scope is authenticated" << endl;
        config_->authenticated = true;
    }

    config_->access_token = auth_data.access_token;
    config_->client_id = auth_data.client_id;
    config_->client_secret = auth_data.client_secret;
}

void Scope::stop() {
}

sc::SearchQueryBase::UPtr Scope::search(const sc::CannedQuery &query,
        const sc::SearchMetadata &metadata) {
    auto client = make_shared<Client>(config_);
    return sc::SearchQueryBase::UPtr(new Query(query, metadata, client));
}

sc::PreviewQueryBase::UPtr Scope::preview(sc::Result const& result,
        sc::ActionMetadata const& metadata) {
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
