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

#ifndef YOUTUBE_API_CONFIG_H_
#define YOUTUBE_API_CONFIG_H_

#include <unity/scopes/OnlineAccountClient.h>

#include <iostream>
#include <memory>
#include <string>
#include <mutex>

namespace youtube {
namespace api {

struct Config {
    typedef std::shared_ptr<Config> Ptr;

    /*
     * The access token provided at instantiation
     */
    std::string access_token { };

    /*
     * The client id provided at instantiation
     */
    std::string client_id { };

    /*
     * The secret provided at instantiation
     */
    std::string client_secret { };

    /*
     * API key for unauthenticated access
     */
    std::string api_key {"AIzaSyDNux7onp1CTDEj99OV0qrVDxe8J3D7KYU"};

    /*
     * The root of all API request URLs
     */
    std::string apiroot {"https://www.googleapis.com"};

    /*
     * The custom HTTP user agent string for this library
     */
    std::string user_agent {"unity-scope-youtube 0.1"};

    /*
     * Default "Accept" HTTP header sent with every request if none is specified
     */
    std::string accept {"application/json"};

    /*
     * Have we got access to private APIs?
     */
    bool authenticated = false;

    /*
     * Update all members to current values
     */
    void update()
    {
        std::lock_guard<std::mutex> lock(config_mutex_);

        if (getenv("YOUTUBE_SCOPE_APIROOT")) {
            apiroot = getenv("YOUTUBE_SCOPE_APIROOT");
        }

        if (getenv("YOUTUBE_SCOPE_IGNORE_ACCOUNTS") != nullptr) {
            return;
        }

        /// TODO: The code commented out below should be removed as soon as
        /// OnlineAccountClient::refresh_service_statuses() is fixed (Bug #1398813).
        /// For now we have to re-instantiate a new OnlineAccountClient each time.

        ///if (oa_client_ == nullptr) {
            oa_client_.reset(
                    new unity::scopes::OnlineAccountClient(SCOPE_INSTALL_NAME,
                            "sharing", "google"));
        ///} else {
        ///    oa_client_->refresh_service_statuses();
        ///}

        for (auto const& status : oa_client_->get_service_statuses()) {
            if (status.service_authenticated) {
                authenticated = true;
                access_token = status.access_token;
                client_id = status.client_id;
                client_secret = status.client_secret;
                break;
            }
        }

        if (!authenticated) {
            access_token = "";
            client_id = "";
            client_secret = "";
            std::cerr << "YouTube scope is unauthenticated" << std::endl;
        } else {
            std::cerr << "YouTube scope is authenticated" << std::endl;
        }
    }

private:
    std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client_ = nullptr;
    std::mutex config_mutex_;
};

}
}

#endif /* YOUTUBE_API_CONFIG_H_ */
