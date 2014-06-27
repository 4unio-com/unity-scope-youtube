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

#ifndef VIMEO_API_SIMPLEOAUTH_H_
#define VIMEO_API_SIMPLEOAUTH_H_

#include <map>
#include <memory>
#include <string>

#include <glib.h>
#include <libaccounts-glib/accounts-glib.h>
#include <libsignon-glib/signon-glib.h>

namespace youtube {
namespace api {

class SimpleOAuth {
public:
    struct AuthData {
        std::string client_id;
        std::string client_secret;
        std::string access_token;
    };

    SimpleOAuth(const std::string &service_name);

    virtual ~SimpleOAuth() = default;

    virtual SimpleOAuth::AuthData auth_data() const;

protected:
    void auth_login(const std::string &clientId, const std::string &clientSecret,
            const std::string &accessToken);

    static void login_cb(GObject *source, GAsyncResult *result,
            void *user_data);

    void login(SignonAuthSession *source, GAsyncResult *result);

    void login_service();

    void logout_service();

    void lookup_account_service();

    static void account_enabled_cb(AgManager *manager, guint account_id,
            void *user_data);

    void account_enabled(AgManager *manager, guint account_id);

    gboolean setup_context();

    static gboolean setup_context_cb(void *user_data);

    std::string service_name_;

    std::shared_ptr<GMainLoop> main_loop_;
    std::shared_ptr<AgManager> manager_;
    std::shared_ptr<AgAccountService> account_service_;
    AgAuthData *ag_auth_data_;
    std::shared_ptr<SignonAuthSession> session_;

    std::shared_ptr<GVariant> auth_params_;
    std::shared_ptr<GVariant> session_data_;

    AuthData auth_data_;
};

}
}

#endif // VIMEO_API_SIMPLEOAUTH_H_
