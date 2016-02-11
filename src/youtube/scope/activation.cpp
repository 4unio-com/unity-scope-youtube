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
 *         Gary Wang  <gary.wang@canonical.com>
 */

#include <boost/algorithm/string.hpp>

#include <youtube/scope/activation.h>
#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/ActionMetadata.h>

#include <iostream>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace youtube::scope;
using namespace youtube::api;

template<typename T>
static T get_or_throw(future<T> &f) {
    if (f.wait_for(std::chrono::seconds(10)) != future_status::ready) {
        throw domain_error("HTTP request timeout");
    }
    return f.get();
}

Activation::Activation(const sc::Result &result,
               const sc::ActionMetadata &metadata,
               std::string const& action_id,
               std::shared_ptr<sc::OnlineAccountClient> oa_client) : 
    sc::ActivationQueryBase(result, metadata), 
    action_id_(action_id),
    client_(oa_client) {
}

sc::ActivationResponse Activation::activate() {
    try {
        string vid = result()["uri"].get_string();
        string fav_listid = result()["fav_playlist"].get_string();
        string watch_listid = result()["watch_playlist"].get_string();

        if (action_id_ == "commented") {
            string comments = action_metadata().scope_data().get_dict()["comment"].get_string();

            future<bool> post_future = client_.post_comments(vid, comments);
            auto status = get_or_throw(post_future);
            cout<< "auth user post a comment: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (action_id_ == "thumb_up") {
            future<bool> like_future = client_.rate(vid, true);
            auto status = get_or_throw(like_future);
            cout<< "auth user likes video: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (action_id_ == "thumb_down") {
            future<bool> ret_future = client_.rate(vid, false);
            auto status = get_or_throw(ret_future);
            cout<< "auth user dislike video: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (action_id_ == "add_fav_list") {
            future<bool> fav_future = client_.addVideoIntoPlayList(vid, fav_listid);
            auto status = get_or_throw(fav_future);
            cout<< "auth user add video in fav list: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (action_id_ == "add_watch_list") {
            future<bool> watch_future = client_.addVideoIntoPlayList(vid, watch_listid);
            auto status = get_or_throw(watch_future);
            cout<< "auth user add video in watch later list: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (alg::starts_with(action_id_,"subscribe:")) {
            auto cid = action_id_.substr(string("subscribe:").length());
            future<bool> subscribe_future = client_.subscribe(cid);
            auto status = get_or_throw(subscribe_future);
            cout<< "auth user subscribe channel: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (alg::starts_with(action_id_,"unsubscribe:")) {
            auto cid = action_id_.substr(string("unsubscribe:").length());
            future<bool> unsubscribe_future = client_.unSubscribe(cid);
            auto status = get_or_throw(unsubscribe_future);
            cout<< "auth user unsubscribe channel: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        }
    }catch (domain_error &e) {
        cerr << e.what() << endl;
    }
    return sc::ActivationResponse(sc::ActivationResponse::Status::NotHandled);
}
