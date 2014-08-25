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

#include <youtube/api/simple-oauth.h>

#include <iostream>

using namespace youtube::api;
using namespace std;

SimpleOAuth::SimpleOAuth(const std::string &service_name) :
        service_name_(service_name) {
    main_loop_.reset(g_main_loop_new(nullptr, true), g_main_loop_unref);

    manager_.reset(ag_manager_new(), g_object_unref);
    g_idle_add(setup_context_cb, this);

    g_main_loop_run(main_loop_.get());
}

SimpleOAuth::AuthData SimpleOAuth::auth_data() const {
    return auth_data_;
}

void SimpleOAuth::auth_login(const string &clientId, const string &clientSecret,
        const string &accessToken) {
    auth_data_.client_id = clientId;
    auth_data_.client_secret = clientSecret;
    auth_data_.access_token = accessToken;

    g_main_loop_quit(main_loop_.get());
}

void SimpleOAuth::login_cb(GObject *source, GAsyncResult *result,
        void *user_data) {
    SignonAuthSession *session = (SignonAuthSession *) source;
    SimpleOAuth *accounts = static_cast<SimpleOAuth*>(user_data);
    accounts->login(session, result);
}

void SimpleOAuth::login(SignonAuthSession *session, GAsyncResult *result) {
    GError *error = nullptr;
    session_data_.reset(
            signon_auth_session_process_finish(session, result, &error),
            g_variant_unref);

    session_.reset();

    if (error) {
        cerr << "Authentication failed: " << error->message << endl;
        g_error_free(error);
        session_data_.reset();
        g_main_loop_quit(main_loop_.get());
        return;
    }

    char *tmp = nullptr;
    string client_id;
    string client_secret;
    string access_token;
    if (g_variant_lookup(auth_params_.get(), "ClientId", "&s", &tmp)) {
        client_id = tmp;
    }
    if (g_variant_lookup(auth_params_.get(), "ClientSecret", "&s", &tmp)) {
        client_secret = tmp;
    }
    if (g_variant_lookup(session_data_.get(), "AccessToken", "&s", &tmp)) {
        access_token = tmp;
    }

    auth_login(client_id, client_secret, access_token);
}

void SimpleOAuth::login_service() {
    ag_auth_data_ = ag_account_service_get_auth_data(account_service_.get());

    GError *error = NULL;
    session_.reset(
            signon_auth_session_new(
                    ag_auth_data_get_credentials_id(ag_auth_data_),
                    ag_auth_data_get_method(ag_auth_data_), &error),
            g_object_unref);
    if (error != NULL) {
        cerr << "Could not set up auth session: " << error->message << endl;
        g_error_free(error);
        g_main_loop_quit(main_loop_.get());
        return;
    }

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}",
    SIGNON_SESSION_DATA_UI_POLICY,
            g_variant_new_int32(SIGNON_POLICY_NO_USER_INTERACTION));

    auth_params_.reset(
            g_variant_ref_sink(
                    ag_auth_data_get_login_parameters(ag_auth_data_,
                            g_variant_builder_end(&builder))), g_variant_unref);

    signon_auth_session_process_async(session_.get(), auth_params_.get(),
            ag_auth_data_get_mechanism(ag_auth_data_),
            NULL, /* cancellable */
            login_cb, this);
}

void SimpleOAuth::logout_service() {
    session_data_.reset();
    auth_params_.reset();
    if (session_) {
        signon_auth_session_cancel(session_.get());
        session_.reset();
    }
    ag_auth_data_ = nullptr;
    account_service_.reset();

    auth_login(string(), string(), string());
}

void SimpleOAuth::lookup_account_service() {
    GList *account_services = ag_manager_get_account_services(
            manager_.get());
    GList *tmp;
    for (tmp = account_services; tmp != nullptr; tmp = tmp->next) {
        AgAccountService *acct_svc = AG_ACCOUNT_SERVICE(tmp->data);
        AgService *service = ag_account_service_get_service(acct_svc);
        if (service_name_ == ag_service_get_name(service)) {
            account_service_.reset(AG_ACCOUNT_SERVICE(g_object_ref(acct_svc)),
                    g_object_unref);
            break;
        }
    }
    ag_manager_list_free(account_services);

    if (account_service_) {
        login_service();
    } else {
        cerr << "Could not find account service" << endl;
        g_main_loop_quit(main_loop_.get());
    }
}

void SimpleOAuth::account_enabled_cb(AgManager *manager, guint account_id,
        void *user_data) {
    SimpleOAuth *accounts = static_cast<SimpleOAuth*>(user_data);
    accounts->account_enabled(manager, account_id);
}

void SimpleOAuth::account_enabled(AgManager *, guint) {
    if (account_service_
            && !ag_account_service_get_enabled(account_service_.get())) {
        logout_service();
    }
    lookup_account_service();
}

gboolean SimpleOAuth::setup_context_cb(void *user_data) {
    SimpleOAuth *accounts = static_cast<SimpleOAuth*>(user_data);
    return accounts->setup_context();
}

gboolean SimpleOAuth::setup_context() {
    lookup_account_service();
    g_signal_connect(manager_.get(), "enabled-event",
            G_CALLBACK(account_enabled_cb), this);
    return FALSE;
}
