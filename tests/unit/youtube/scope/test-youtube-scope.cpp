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

#include <core/posix/exec.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchReplyProxyFwd.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/testing/Category.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/TypedScopeFixture.h>

using namespace std;
using namespace testing;
using namespace youtube::scope;

namespace posix = core::posix;
namespace sc = unity::scopes;
namespace sct = unity::scopes::testing;

namespace {

MATCHER_P2(ResultProp, prop, value, "") {
    if (arg.contains(prop)) {
        *result_listener << "result[" << prop << "] is " << arg[prop].serialize_json();
    } else {
        *result_listener << "result[" << prop << "] is not set";
    }
    return arg.contains(prop) && arg[prop] == sc::Variant(value);
}

MATCHER_P(IsDepartment, department, "") {
    return arg->serialize() == department->serialize();
}

typedef sct::TypedScopeFixture<Scope> TypedScopeFixtureScope;

class TestYoutubeScope: public TypedScopeFixtureScope {
protected:
    void SetUp() override
    {
        fake_youtube_server_ = posix::exec(FAKE_YOUTUBE_SERVER, { }, { },
                posix::StandardStream::stdout);

        ASSERT_GT(fake_youtube_server_.pid(), 0);
        string port;
        fake_youtube_server_.cout() >> port;

        string apiroot = "http://127.0.0.1:" + port;
        setenv("YOUTUBE_SCOPE_APIROOT", apiroot.c_str(), true);

        setenv("YOUTUBE_SCOPE_IGNORE_ACCOUNTS", "true", true);

        // Do the parent SetUp
        TypedScopeFixtureScope::SetUp();
    }

    posix::ChildProcess fake_youtube_server_ = posix::ChildProcess::invalid();
};

TEST_F(TestYoutubeScope, non_empty_query) {
    const sc::CategoryRenderer renderer;
    NiceMock<sct::MockSearchReply> reply;

    sc::CannedQuery query("unity-scope-youtube", "banana", ""); // searching with query text

    EXPECT_CALL(reply, register_category("youtube", "1000000 results from Youtube", "", _)).Times(1)
                .WillOnce(Return(make_shared<sct::Category>("youtube", "1000000 results from Youtube", "", renderer)));

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "gLPKjkXsWM8"),
            ResultProp("link", "https://www.youtube.com/watch?v=gLPKjkXsWM8"),
            ResultProp("title", "Harry eating a banana on stage (Düsseldorf, Germany) HD"),
            ResultProp("art", "https://i.ytimg.com/vi/gLPKjkXsWM8/hqdefault.jpg"),
            ResultProp("description", "One Direction at the Where We Are Tour in Düsseldorf, Germany on the 2nd of July 2014. HD."),
            ResultProp("username", ""))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "ZYXTZh8CW4E"),
            ResultProp("link", "https://www.youtube.com/watch?v=ZYXTZh8CW4E"),
            ResultProp("title", "Minions - Banana 14:20 mins"),
            ResultProp("art", "https://i.ytimg.com/vi/ZYXTZh8CW4E/hqdefault.jpg"),
            ResultProp("description", "Despicable me 2 Minions Banana song."),
            ResultProp("username", "ohvsk8"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "BYBw_o_2nG0"),
            ResultProp("link", "https://www.youtube.com/watch?v=BYBw_o_2nG0"),
            ResultProp("title", "Despicable Me - Mini-Movie 'Banana' Preview"),
            ResultProp("art", "https://i.ytimg.com/vi/BYBw_o_2nG0/hqdefault.jpg"),
            ResultProp("description", ""),
            ResultProp("username", "DespicableMeMovie"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "FQymDE3FaHY"),
            ResultProp("link", "https://www.youtube.com/watch?v=FQymDE3FaHY"),
            ResultProp("title", "Tutorial: Anti-rush flashbang @ de_inferno banana"),
            ResultProp("art", "https://i.ytimg.com/vi/FQymDE3FaHY/hqdefault.jpg"),
            ResultProp("description", "Made by: https://www.youtube.com/user/ThePavle995 » Liked this video? Subscribe for more: http://youtube.com/subscription_center?add_user=piccgamer ..."),
            ResultProp("username", "piccgamer"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "Z01ts2f-mHY"),
            ResultProp("link", "https://www.youtube.com/watch?v=Z01ts2f-mHY"),
            ResultProp("title", "Banana (香蕉人)"),
            ResultProp("art", "https://i.ytimg.com/vi/Z01ts2f-mHY/hqdefault.jpg"),
            ResultProp("description", "When you can't understand something, don't act like you can! :) SUBSCRIBE TO US! DanKhooProductions' FB - http://fb.com/dankhooproductions ..."),
            ResultProp("username", "DanKhooProductions"))))).WillOnce(
            Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_EN", "phone");
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(reply_proxy);
}

} // namespace
