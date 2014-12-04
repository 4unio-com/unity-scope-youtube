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

namespace sc = unity::scopes;
using namespace std;
using namespace youtube::scope;
using namespace youtube::api;

void Scope::start(string const&) {
    setlocale(LC_ALL, "");
    string translation_directory = ScopeBase::scope_directory()
            + "/../share/locale/";
    bindtextdomain(GETTEXT_PACKAGE, translation_directory.c_str());

    if (getenv("YOUTUBE_SCOPE_IGNORE_ACCOUNTS") == nullptr) {
        oa_client_.reset(
                new unity::scopes::OnlineAccountClient(SCOPE_INSTALL_NAME,
                        "sharing", "google"));
    }
}

void Scope::stop() {
}

sc::SearchQueryBase::UPtr Scope::search(const sc::CannedQuery &query,
        const sc::SearchMetadata &metadata) {
    return sc::SearchQueryBase::UPtr(new Query(query, metadata, oa_client_));
}

sc::PreviewQueryBase::UPtr Scope::preview(sc::Result const& result,
        sc::ActionMetadata const& metadata) {
    return sc::PreviewQueryBase::UPtr(new Preview(result, metadata, oa_client_));
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
