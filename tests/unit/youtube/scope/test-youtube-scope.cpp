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
#include <iostream>
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

static const char* SCOPE_ID { "unity-scope-youtube" };

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

    void add_department(const sc::Department::SPtr &departments,
            const string &id, const string &title,
            const sc::CannedQuery &query) {
        sc::Department::SPtr dept = sc::Department::create("guideCategory:" + id,
                query, title);
        dept->add_subdepartment(
                sc::Department::create("guideCategory-videos:" + id, query,
                        "Videos"));
        dept->add_subdepartment(
                sc::Department::create("guideCategory-playlists:" + id, query,
                        "Playlists"));
        dept->add_subdepartment(
                sc::Department::create("guideCategory-channels:" + id, query,
                        "Channels"));
        departments->add_subdepartment(dept);
    }

    void expect_category(NaggyMock<sct::MockSearchReply> &reply,
            const sc::CategoryRenderer &renderer, const string &id,
            const string &title) {
        EXPECT_CALL(reply, register_category(_, title, "", _)).Times(1).WillOnce(
                Return(make_shared<sct::Category>(id, title, "", renderer)));
    }

    posix::ChildProcess fake_youtube_server_ = posix::ChildProcess::invalid();
};

TEST_F(TestYoutubeScope, non_empty_query) {
    const sc::CategoryRenderer renderer;
    NaggyMock<sct::MockSearchReply> reply;

    sc::CannedQuery query(SCOPE_ID, "banana", ""); // searching with query text

    expect_category(reply, renderer, "youtube", "1000000 results from Youtube");

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "gLPKjkXsWM8"),
        ResultProp("link", "https://www.youtube.com/watch?v=gLPKjkXsWM8"),
        ResultProp("title", "Harry eating a banana on stage (Düsseldorf, Germany) HD"),
        ResultProp("art", "https://i.ytimg.com/vi/gLPKjkXsWM8/hqdefault.jpg"),
        ResultProp("description", "One Direction at the Where We Are Tour in Düsseldorf, Germany on the 2nd of July 2014. HD."),
        ResultProp("subtitle", ""))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "ZYXTZh8CW4E"),
        ResultProp("link", "https://www.youtube.com/watch?v=ZYXTZh8CW4E"),
        ResultProp("title", "Minions - Banana 14:20 mins"),
        ResultProp("art", "https://i.ytimg.com/vi/ZYXTZh8CW4E/hqdefault.jpg"),
        ResultProp("description", "Despicable me 2 Minions Banana song."),
        ResultProp("subtitle", "ohvsk8"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "BYBw_o_2nG0"),
        ResultProp("link", "https://www.youtube.com/watch?v=BYBw_o_2nG0"),
        ResultProp("title", "Despicable Me - Mini-Movie 'Banana' Preview"),
        ResultProp("art", "https://i.ytimg.com/vi/BYBw_o_2nG0/hqdefault.jpg"),
        ResultProp("description", ""),
        ResultProp("subtitle", "DespicableMeMovie"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "FQymDE3FaHY"),
        ResultProp("link", "https://www.youtube.com/watch?v=FQymDE3FaHY"),
        ResultProp("title", "Tutorial: Anti-rush flashbang @ de_inferno banana"),
        ResultProp("art", "https://i.ytimg.com/vi/FQymDE3FaHY/hqdefault.jpg"),
        ResultProp("description", "Made by: https://www.youtube.com/user/ThePavle995 » Liked this video? Subscribe for more: http://youtube.com/subscription_center?add_user=piccgamer ..."),
        ResultProp("subtitle", "piccgamer"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "Z01ts2f-mHY"),
        ResultProp("link", "https://www.youtube.com/watch?v=Z01ts2f-mHY"),
        ResultProp("title", "Banana (香蕉人)"),
        ResultProp("art", "https://i.ytimg.com/vi/Z01ts2f-mHY/hqdefault.jpg"),
        ResultProp("description", "When you can't understand something, don't act like you can! :) SUBSCRIBE TO US! DanKhooProductions' FB - http://fb.com/dankhooproductions ..."),
        ResultProp("subtitle", "DanKhooProductions"))))).WillOnce(
    Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_EN", "phone");
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(reply_proxy);
}

TEST_F(TestYoutubeScope, basic_surfacing) {
    const sc::CategoryRenderer renderer;
    NaggyMock<sct::MockSearchReply> reply;

    sc::CannedQuery query(SCOPE_ID, "", ""); // welcome screen

    sc::Department::SPtr departments = sc::Department::create("", query,
            "Best of YouTube");
    add_department(departments, "GCRmVhdHVyZWQ", "Featured", query);
    add_department(departments, "GCUGFpZCBDaGFubmVscw", "Paid Channels", query);
    add_department(departments, "GCTXVzaWM", "Music", query);
    add_department(departments, "GCQ29tZWR5", "Comedy", query);
    EXPECT_CALL(reply, register_departments(IsDepartment(departments))).Times(
            1);

    expect_category(reply, renderer, "youtube-popular", "");

    expect_category(reply, renderer, "UCF0pVplsI8R5kcAqgtoRqoA", "Popular on YouTube");
    expect_category(reply, renderer, "UC3yA8nDwraeOfnYfBWun83g", "Education");
    expect_category(reply, renderer, "UC-9-kyTW8ZkZNDHQJ6FgpwQ", "Music");
    expect_category(reply, renderer, "UCEgdi0XIXXZ-qJOFPf4JSKw", "Sports");
    expect_category(reply, renderer, "UCOpNcN46UbXVtpKMrmU4Abg", "Gaming");

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "32FbVRrVVNE"),
            ResultProp("title", "This Is How We Roll featuring Halle Berry"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/32FbVRrVVNE/hqdefault.jpg"),
            ResultProp("subtitle", "Popular on YouTube")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "qhbliUq0_r4"),
            ResultProp("title", "LEGO: Everything is NOT awesome."),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/qhbliUq0_r4/hqdefault.jpg"),
            ResultProp("subtitle", "Popular on YouTube")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "NefqXlOmPPA"),
            ResultProp("title", "Frozen is the new Black"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/NefqXlOmPPA/hqdefault.jpg"),
            ResultProp("subtitle", "Popular on YouTube")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "42uphHWxmFU"),
            ResultProp("title", "Florida Georgia Line - Dirt"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/42uphHWxmFU/hqdefault.jpg"),
            ResultProp("subtitle", "Popular on YouTube")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "LqbYVr5jBVk"),
            ResultProp("title", "Tiny Birthday For A Tiny Hedgehog"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/LqbYVr5jBVk/hqdefault.jpg"),
            ResultProp("subtitle", "Popular on YouTube")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "L932sS20Tek"),
            ResultProp("title", "Magnesium Disorders"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/L932sS20Tek/hqdefault.jpg"),
            ResultProp("subtitle", "Education")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "ryA8PafooQ4"),
            ResultProp("title", "Is Evil Rational?"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/ryA8PafooQ4/hqdefault.jpg"),
            ResultProp("subtitle", "Education")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "HtGgreHgvfo"),
            ResultProp("title", "Political Parties Rap (Epic Remix) - Smart Songs"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/HtGgreHgvfo/hqdefault.jpg"),
            ResultProp("subtitle", "Education")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "AWoqTGEw7WA"),
            ResultProp("title", "3D Scanning at the Smithsonian"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/AWoqTGEw7WA/hqdefault.jpg"),
            ResultProp("subtitle", "Education")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "YHdOiTUwdKg"),
            ResultProp("title", "Age of star clusters & Red Giants"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/YHdOiTUwdKg/hqdefault.jpg"),
            ResultProp("subtitle", "Education")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "7-7knsP2n5w"),
            ResultProp("title", "Shakira - La La La (Brazil 2014) ft. Carlinhos Brown"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/7-7knsP2n5w/hqdefault.jpg"),
            ResultProp("subtitle", "Music")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "TGtWWb9emYI"),
            ResultProp("title", "We Are One (Ole Ola) [The Official 2014 FIFA World Cup Song] (Olodum Mix)"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/TGtWWb9emYI/hqdefault.jpg"),
            ResultProp("subtitle", "Music")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "HkMNOlYcpHg"),
            ResultProp("title", "PSY - HANGOVER feat. Snoop Dogg M/V"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/HkMNOlYcpHg/hqdefault.jpg"),
            ResultProp("subtitle", "Music")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "0KSOMA3QBU0"),
            ResultProp("title", "Katy Perry - Dark Horse (Official) ft. Juicy J"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/0KSOMA3QBU0/hqdefault.jpg"),
            ResultProp("subtitle", "Music")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "NUsoVlDFqZg"),
            ResultProp("title", "Enrique Iglesias - Bailando (Español) ft. Descemer Bueno, Gente De Zona"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/NUsoVlDFqZg/hqdefault.jpg"),
            ResultProp("subtitle", "Music")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "QE-Cj_iYEYU"),
            ResultProp("title", "England v Uruguay training | Inside Access"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/QE-Cj_iYEYU/hqdefault.jpg"),
            ResultProp("subtitle", "Sports")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "OkTozcmlDRo"),
            ResultProp("title", "Ross Taylor 42 unbeaten.. patiently played when his team fell apart"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/OkTozcmlDRo/hqdefault.jpg"),
            ResultProp("subtitle", "Sports")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "EnaPvWW3Nco"),
            ResultProp("title", "GTA 5 Online Sticky Bomb Glitch - Carlos vs. Stripper, Floating Bong! (GTA 5 Funny Moments)"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/EnaPvWW3Nco/hqdefault.jpg"),
            ResultProp("subtitle", "Gaming")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "JzDUFe-njT4"),
            ResultProp("title", "Minecraft: REDISCOVERED (SECRET MINECRAFT FEATURES!) Mod Showcase"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/JzDUFe-njT4/hqdefault.jpg"),
            ResultProp("subtitle", "Gaming")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "wxBYgc89C74"),
            ResultProp("title", "Minecraft Mini-Game : DO NOT LAUGH 6!"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/wxBYgc89C74/hqdefault.jpg"),
            ResultProp("subtitle", "Gaming")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "U2cGvE0B1GI"),
            ResultProp("title", "♫  Top 10 Minecraft Songs 2014 July   Minecraft Hunger Games   Minecraft Song"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/U2cGvE0B1GI/hqdefault.jpg"),
            ResultProp("subtitle", "Gaming")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "Bc_PlkdkbS8"),
            ResultProp("title", "\"DANGER ZONE 3\" | GTA 5 Jet Stunt Montage #5"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/Bc_PlkdkbS8/hqdefault.jpg"),
            ResultProp("subtitle", "Gaming")
        )))).WillOnce(Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_EN", "phone");
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(reply_proxy);
}

} // namespace
