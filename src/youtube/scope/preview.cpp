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
#include <boost/algorithm/string/replace.hpp>

#include <youtube/scope/localisation.h>
#include <youtube/scope/preview.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>
#include <sstream>
#include <unordered_set>

namespace sc = unity::scopes;

using namespace std;
using namespace youtube::scope;
using namespace youtube::api;

extern string format_fixed(long long number);

namespace {
static const unordered_set<string> PLAYABLE = { "youtube#video",
        "youtube#playlistItem" };

template<typename T>
static T get_or_throw(future<T> &f) {
    if (f.wait_for(std::chrono::seconds(10)) != future_status::ready) {
        throw domain_error("HTTP request timeout");
    }
    return f.get();
}
}

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata,
                 std::shared_ptr<sc::OnlineAccountClient> oa_client) :
        sc::PreviewQueryBase(result, metadata),
        client_(oa_client) {
}

void Preview::cancelled() {
}

void Preview::playable(const sc::PreviewReplyProxy& reply) {
    auto videos_future = client_.videos(result().uri());
    auto videos = videos_future.get();
    auto v = videos.front();
    auto s = v->statistics();
    auto cid = v->channelId();

    sc::PreviewWidgetList widgets;
    std::vector<std::string> ids;

    sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);
    ids = std::vector<std::string>{ "video", "header", "expandable", "summary", "actions"};

    sc::PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");
    header.add_attribute_value("subtitle", sc::Variant(format_fixed(s.view_count) + _(" views") +
                                                       u8"   \u261d " + format_fixed(s.like_count) +
                                                       u8"   \u261f " + format_fixed(s.dislike_count) +
                                                       u8"   \u270E " + format_fixed(s.comment_count)));
    widgets.emplace_back(header);

    sc::PreviewWidget video("video", "video");
    video.add_attribute_mapping("source", "link");
    video.add_attribute_mapping("screenshot", "art");
    sc::VariantMap share_data;
    share_data["uri"] = result()["link"];
    share_data["content-type"] = sc::Variant("videos");
    video.add_attribute_value("share-data", sc::Variant(share_data));
    widgets.emplace_back(video);

    sc::PreviewWidget w_expandable("expandable", "expandable");
    w_expandable.add_attribute_value("collapsed-widgets", sc::Variant(2));
    sc::PreviewWidget w_publish("publish", "text");
    string publishInfo = "<b>" + v->username() +_("<br/>  Published on: </b>") + v->publishedAt();

    if(!client_.authenticated()) {
        ids.emplace_back("tips-id");
        sc::PreviewWidget w_tips(ids.at(ids.size() - 1), "text");
        w_tips.add_attribute_value("text", sc::Variant(_("Please login to post a comment  ")));
        widgets.emplace_back(w_tips);

        sc::PreviewWidget w_action("actions", "actions");
        sc::VariantBuilder builder;
        builder.add_tuple({
              {"id", sc::Variant(_("open"))},
              {"label", sc::Variant(_("Login to youtube"))},
          });

        sc::OnlineAccountClient oa_client(SCOPE_INSTALL_NAME, "sharing", "google");

        oa_client.register_account_login_item(w_action,
                                              sc::OnlineAccountClient::InvalidateResults,
                                              sc::OnlineAccountClient::DoNothing);

        w_action.add_attribute_value("actions", builder.end());
        widgets.emplace_back(w_action);
    } else {
        sc::VariantBuilder builder;
        sc::PreviewWidget actions("actions", "actions");
        {
            auto subscribed_future = client_.subscribeId(cid);
            auto subsribedList = get_or_throw(subscribed_future);

            builder.add_tuple({
                  {"id", sc::Variant(subsribedList.size() > 0 ?
                   "unsubscribe:" + subsribedList[0]->subscribeId() : "subscribe:" + cid)},
                  {"label", sc::Variant(_(subsribedList.size() > 0 ? _("Unsubscribe"): _("Subscribe")))}
              });
            builder.add_tuple({
                  {"id", sc::Variant("thumb_up")},
                  {"label", sc::Variant(_("Thumb up"))}
              });
            builder.add_tuple({
                  {"id", sc::Variant("thumb_down")},
                  {"label", sc::Variant(_("Thumb down"))}
              });
            builder.add_tuple({
                  {"id", sc::Variant("add_watch_list")},
                  {"label", sc::Variant(_("Add to watch list"))}
              });
            builder.add_tuple({
                  {"id", sc::Variant("add_fav_list")},
                  {"label", sc::Variant(_("Add to favorites list"))}
              });
	    sc::CannedQuery new_query(SCOPE_INSTALL_NAME);
            new_query.set_department_id("channel:"+cid);
            builder.add_tuple({
                {"id", sc::Variant("user_channel")},
                {"label", sc::Variant(_("User channel"))},
                {"uri", sc::Variant(new_query.to_uri())}
            });
        }
        actions.add_attribute_value("actions", builder.end());
        widgets.emplace_back(actions);

        ids.emplace_back("comment-inputid");
        sc::PreviewWidget w_commentInput(ids.at(ids.size() - 1), "comment-input");
        w_commentInput.add_attribute_value("submit-label", sc::Variant(_("Post")));
        widgets.emplace_back(w_commentInput);

        int index = 0;
        auto commentlist_future = client_.video_comments(v->id());
        for (const auto &comment : get_or_throw(commentlist_future)) {
            std::string id = "commentId_"+ std::to_string(index++);
            ids.emplace_back(id);

            sc::PreviewWidget w_comment(id, "comment");
            w_comment.add_attribute_value("comment", sc::Variant(comment->body()));
            w_comment.add_attribute_value("author", sc::Variant(comment->title()));
            w_comment.add_attribute_value("source", sc::Variant(comment->picture()));
            w_comment.add_attribute_value("subtitle", sc::Variant(comment->created_at()));
            widgets.emplace_back(w_comment);
        }
    }

    w_publish.add_attribute_value("text", sc::Variant(publishInfo));
    w_expandable.add_widget(w_publish);

    //Split into half if description is too long
    sc::PreviewWidget w_description("summary1", "text");
    string desc = result()["description"].get_string();
    const int TEXT_LIMIT_LENGTH = 100;
    if (desc.length() < TEXT_LIMIT_LENGTH) {
        w_description.add_attribute_value("text", sc::Variant(desc));
        w_expandable.add_widget(w_description);
    } else {
        sc::PreviewWidget w_description2("summary2", "text");
        string desc1 = desc.substr(0, TEXT_LIMIT_LENGTH);
        string desc2 = desc.substr(TEXT_LIMIT_LENGTH);

        w_description.add_attribute_value("text", sc::Variant(desc1));
        w_expandable.add_widget(w_description);
        w_description2.add_attribute_value("text", sc::Variant(desc2));
        w_expandable.add_widget(w_description2);
    }

    widgets.emplace_back(w_expandable);

    layout1col.add_column(ids);

    std::vector<std::string> ids2(ids.size() - 1);
    std::copy(ids.begin() + 1, ids.end(), ids2.begin());

    layout2col.add_column({ "video" });
    layout2col.add_column(ids);

    layout3col.add_column({ "video" });
    layout3col.add_column(ids2);
    layout3col.add_column({""});

    reply->register_layout( { layout1col, layout2col, layout3col });
    reply->push(widgets);
}

void Preview::playlist(const sc::PreviewReplyProxy& reply) {
    sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);
    layout1col.add_column( { "image", "header", "summary", "actions" });
    layout2col.add_column( { "image" });
    layout2col.add_column( { "header", "summary", "actions" });
    layout3col.add_column( { "image" });
    layout3col.add_column( { "header", "summary" });
    layout3col.add_column( { "actions" });
    reply->register_layout( { layout1col, layout2col, layout3col });

    sc::PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "subtitle");

    sc::PreviewWidget image("image", "image");
    image.add_attribute_mapping("source", "art");
    sc::VariantMap share_data;
    share_data["uri"] = result()["art"];
    share_data["content-type"] = sc::Variant("pictures");
    image.add_attribute_value("share-data", sc::Variant(share_data));

    sc::PreviewWidget description("summary", "text");
    description.add_attribute_mapping("text", "description");

    sc::PreviewWidget actions("actions", "actions");
    {
        sc::VariantBuilder builder;
        builder.add_tuple( { { "id", sc::Variant("search") }, { "uri",
                sc::Variant(result().uri()) },
                { "label", sc::Variant("Search") } });
        actions.add_attribute_value("actions", builder.end());
    }

    reply->push( { image, header, description, actions });
}

void Preview::userInfo(const sc::PreviewReplyProxy& reply) {
    sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);
    layout1col.add_column( { "header", "art", "statistics", "description", "actions" });
    layout2col.add_column( { "header" });
    layout2col.add_column( { "art", "statistics", "description", "actions" });
    layout3col.add_column( { "header" });
    layout3col.add_column( { "art", "statistics", "description" });
    layout3col.add_column( { "actions" });
    reply->register_layout( { layout1col, layout2col, layout3col });

    sc::PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");

    string artwork_url = result()["art"].get_string();
    boost::replace_all(artwork_url, "s88-c-k-no", "s240-c-k-no");
    sc::PreviewWidget art("art", "image");
    art.add_attribute_value("source", sc::Variant(artwork_url));
    sc::VariantMap share_data;
    share_data["uri"] = result()["art"];
    share_data["content-type"] = sc::Variant("pictures");
    art.add_attribute_value("share-data", sc::Variant(share_data));

    sc::PreviewWidget statistics("statistics", "header");
    statistics.add_attribute_value("title", sc::Variant(
           result()["videos-count"].get_string() + "  " +
           result()["views-count"].get_string() + "  " +
           result()["subscribers-count"].get_string()));
    sc::PreviewWidget description("description", "text");
    description.add_attribute_mapping("text", "desc");

    auto cid = result()["uri"].get_string();
    sc::VariantBuilder builder;
    sc::PreviewWidget actions("actions", "actions");
    {
        builder.add_tuple({
              {"id", sc::Variant("view")},
              {"label", sc::Variant(_("View in browser"))},
              {"uri", sc::Variant("https://www.youtube.com/channel/" + cid)}
          });
        sc::CannedQuery new_query(SCOPE_INSTALL_NAME);
        new_query.set_department_id("channel:"+cid);
        builder.add_tuple({
              {"id", sc::Variant("user_channel")},
              {"label", sc::Variant(_("User channel"))},
              {"uri", sc::Variant(new_query.to_uri())}
        });
    }
    actions.add_attribute_value("actions", builder.end());

    reply->push( { header, art, statistics, description, actions});
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    string kind = result()["kind"].get_string();

    if (kind == "user-info"){
        userInfo(reply);
    } else if (PLAYABLE.find(kind) == PLAYABLE.end()) {
        playlist(reply);
    } else {
        playable(reply);
    }
}
