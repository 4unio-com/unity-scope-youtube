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
#include <unity/scopes/testing/ScopeMetadataBuilder.h>

using namespace std;
using namespace testing;
using namespace youtube::scope;

namespace posix = core::posix;
namespace sc = unity::scopes;
namespace sct = unity::scopes::testing;

namespace unity {
namespace scopes {

void PrintTo(const CategorisedResult& res, ::std::ostream* os) {
  *os << "CategorisedResult: " << Variant(res.serialize()).serialize_json();
}

}
}

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

MATCHER_P(ResultUriMatchesCannedQuery, q, "") {
    auto const query = unity::scopes::CannedQuery::from_uri(arg.uri());
    return query.scope_id() == q.scope_id()
        && query.query_string() == q.query_string()
        && query.department_id() == q.department_id();
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
        TypedScopeFixture::set_scope_directory(TEST_SCOPE_DIRECTORY);
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
        EXPECT_CALL(reply, register_category(id, title, "", _)).Times(1).WillOnce(
                Return(make_shared<sct::Category>(id, title, "", renderer)));
    }

    posix::ChildProcess fake_youtube_server_ = posix::ChildProcess::invalid();
};

TEST_F(TestYoutubeScope, non_empty_query) {
    const sc::CategoryRenderer renderer;
    NaggyMock<sct::MockSearchReply> reply;

    sc::CannedQuery query(SCOPE_NAME, "banana", ""); // searching with query text

    expect_category(reply, renderer, "youtube", "1000000 results from YouTube");

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "gLPKjkXsWM8"),
        ResultProp("link", "http://www.youtube.com/watch?v=gLPKjkXsWM8"),
        ResultProp("title", "Harry eating a banana on stage (Düsseldorf, Germany) HD"),
        ResultProp("art", "https://i.ytimg.com/vi/gLPKjkXsWM8/hqdefault.jpg"),
        ResultProp("description", "One Direction at the Where We Are Tour in Düsseldorf, Germany on the 2nd of July 2014. HD."),
        ResultProp("subtitle", ""))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "ZYXTZh8CW4E"),
        ResultProp("link", "http://www.youtube.com/watch?v=ZYXTZh8CW4E"),
        ResultProp("title", "Minions - Banana 14:20 mins"),
        ResultProp("art", "https://i.ytimg.com/vi/ZYXTZh8CW4E/hqdefault.jpg"),
        ResultProp("description", "Despicable me 2 Minions Banana song."),
        ResultProp("subtitle", "ohvsk8"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "BYBw_o_2nG0"),
        ResultProp("link", "http://www.youtube.com/watch?v=BYBw_o_2nG0"),
        ResultProp("title", "Despicable Me - Mini-Movie 'Banana' Preview"),
        ResultProp("art", "https://i.ytimg.com/vi/BYBw_o_2nG0/hqdefault.jpg"),
        ResultProp("description", ""),
        ResultProp("subtitle", "DespicableMeMovie"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "FQymDE3FaHY"),
        ResultProp("link", "http://www.youtube.com/watch?v=FQymDE3FaHY"),
        ResultProp("title", "Tutorial: Anti-rush flashbang @ de_inferno banana"),
        ResultProp("art", "https://i.ytimg.com/vi/FQymDE3FaHY/hqdefault.jpg"),
        ResultProp("description", "Made by: https://www.youtube.com/user/ThePavle995 » Liked this video? Subscribe for more: http://youtube.com/subscription_center?add_user=piccgamer ..."),
        ResultProp("subtitle", "piccgamer"))))).WillOnce(
            Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("uri", "Z01ts2f-mHY"),
        ResultProp("link", "http://www.youtube.com/watch?v=Z01ts2f-mHY"),
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

    sc::CannedQuery query(SCOPE_NAME, "", ""); // welcome screen

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

    expect_category(reply, renderer, "youtube_login_nag", "");
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(
        ResultProp("title", "Log-in to YouTube")
        ))).WillOnce(
    Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_EN", "phone");
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(reply_proxy);
}

TEST_F(TestYoutubeScope, pick_department) {
    const sc::CategoryRenderer renderer;
    NaggyMock<sct::MockSearchReply> reply;

    sc::CannedQuery query(SCOPE_NAME, "", "guideCategory:GCTXVzaWM"); // pick the music department

    sc::Department::SPtr departments = sc::Department::create("", query,
            "Best of YouTube");
    add_department(departments, "GCRmVhdHVyZWQ", "Featured", query);
    add_department(departments, "GCUGFpZCBDaGFubmVscw", "Paid Channels", query);
    add_department(departments, "GCTXVzaWM", "Music", query);
    add_department(departments, "GCQ29tZWR5", "Comedy", query);
    EXPECT_CALL(reply, register_departments(IsDepartment(departments))).Times(
            1);

    expect_category(reply, renderer, "youtube-popular", "");

    expect_category(reply, renderer, "UCdI8evszfZvyAl2UVCypkTA", "MileyCyrusVEVO");
    expect_category(reply, renderer, "UC_TVqp_SyG6j5hG-xVRy95A", "Skrillex");
    expect_category(reply, renderer, "UC20vb-R_px4CguHzzBPhoyQ", "EminemVEVO");
    expect_category(reply, renderer, "UCpDJl2EmP7Oh90Vylx0dZtA", "Spinnin' Records");
    expect_category(reply, renderer, "UCrDkAvwZum-UTjHmzDI2iIw", "officialpsy");

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "0EdAYgjthWQ"),
            ResultProp("title", "#VEVOCertified, Pt 1:  Miley Talks About Her Fans"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/0EdAYgjthWQ/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "ukF1t-PeZWE"),
            ResultProp("title", "Miley Cyrus - #VEVOCertified, Pt 2:  Award Presentation"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/ukF1t-PeZWE/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "NThkyod_A0A"),
            ResultProp("title", "#VEVOCertified, Pt 3: Miley On Making Music Videos"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/NThkyod_A0A/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "gf4p-nJ4BmM"),
            ResultProp("title", "#VEVOCertified, Pt 4: Wrecking Ball (Miley Commentary)"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/gf4p-nJ4BmM/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "OLxsBS-tI5k"),
            ResultProp("title", "#VEVOCertified, Pt 5: We Can't Stop (Miley Commentary)"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/OLxsBS-tI5k/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "6JYIGclVQdw"),
            ResultProp("title", "Skrillex - All Is Fair in Love and Brostep with Ragga Twins [AUDIO]"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/6JYIGclVQdw/hqdefault.jpg"),
            ResultProp("subtitle", "Skrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "R8i6VZ1vIY8"),
            ResultProp("title", "Skrillex - Recess with Kill the Noise, Fatman Scoop, and Michael Angelakos [AUDIO]"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/R8i6VZ1vIY8/hqdefault.jpg"),
            ResultProp("subtitle", "Skrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "6t7wQ2BLBUg"),
            ResultProp("title", "Skrillex - Stranger with KillaGraham from Milo and Otis and Sam Dew [AUDIO]"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/6t7wQ2BLBUg/hqdefault.jpg"),
            ResultProp("subtitle", "Skrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "nOqUvLmwfTg"),
            ResultProp("title", "Skrillex & Alvin Risk - Try It Out (Neon Mix) [AUDIO]"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/nOqUvLmwfTg/hqdefault.jpg"),
            ResultProp("subtitle", "Skrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "Nu3cezCkzf4"),
            ResultProp("title", "Skrillex - Coast Is Clear with Chance The Rapper and the Social Experiment [AUDIO]"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/Nu3cezCkzf4/hqdefault.jpg"),
            ResultProp("subtitle", "Skrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "O-zpOMYRi0w"),
            ResultProp("title", "Iggy Azalea - Fancy (Explicit) ft. Charli XCX"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/O-zpOMYRi0w/hqdefault.jpg"),
            ResultProp("subtitle", "Vevo")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "2iZonnn1nIs"),
            ResultProp("title", "Phife - Dear Dilla"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/2iZonnn1nIs/hqdefault.jpg"),
            ResultProp("subtitle", "Vevo")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "rEMsjeq43_U"),
            ResultProp("title", "ScHoolboy Q - Man Of The Year"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/rEMsjeq43_U/hqdefault.jpg"),
            ResultProp("subtitle", "Vevo")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "cI9ifCMk0Dg"),
            ResultProp("title", "Rick Ross - Nobody (Explicit) ft. French Montana, Puff Daddy"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/cI9ifCMk0Dg/hqdefault.jpg"),
            ResultProp("subtitle", "Vevo")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "6l7J1i1OkKs"),
            ResultProp("title", "YG - My Nigga (Remix) (Explicit) ft. Lil Wayne, Rich Homie Quan, Meek Mill, Nicki Minaj"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/6l7J1i1OkKs/hqdefault.jpg"),
            ResultProp("subtitle", "Vevo")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "zrEucH0E8Ic"),
            ResultProp("title", "Spinnin' Sessions 061 - Guest: Sander Kleinenberg"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/zrEucH0E8Ic/hqdefault.jpg"),
            ResultProp("subtitle", "Spinnin' Records")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "YuWfQPJoLw0"),
            ResultProp("title", "Ferreck Dawn & Redondo - Love Too Deep (Official Music Video)"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/YuWfQPJoLw0/hqdefault.jpg"),
            ResultProp("subtitle", "Spinnin' Records")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "fMP8xZdKs4Y"),
            ResultProp("title", "DubVision - Backlash (Martin Garrix Edit) [Official Music Video]"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/fMP8xZdKs4Y/hqdefault.jpg"),
            ResultProp("subtitle", "Spinnin' Records")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "e15Qwu5Z390"),
            ResultProp("title", "Nick Double & Sam O Neall - Live Life (Official Lyric Video)"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/e15Qwu5Z390/hqdefault.jpg"),
            ResultProp("subtitle", "Spinnin' Records")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "Ykj_wJKkaAg"),
            ResultProp("title", "Ummet Ozcan - SMASH! (Official Music Video)"),
            ResultProp("kind", "youtube#playlistItem"),
            ResultProp("art", "https://i1.ytimg.com/vi/Ykj_wJKkaAg/hqdefault.jpg"),
            ResultProp("subtitle", "Spinnin' Records")
        )))).WillOnce(Return(true));

    expect_category(reply, renderer, "youtube_login_nag", "");
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(
        ResultProp("title", "Log-in to YouTube")
        ))).WillOnce(
    Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_EN", "phone");
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(reply_proxy);
}

TEST_F(TestYoutubeScope, pick_department_channels) {
    const sc::CategoryRenderer renderer;
    NaggyMock<sct::MockSearchReply> reply;

    sc::CannedQuery query(SCOPE_NAME, "", "guideCategory-channels:GCTXVzaWM"); // pick the music channels department

    sc::Department::SPtr departments = sc::Department::create("", query,
            "Best of YouTube");
    add_department(departments, "GCRmVhdHVyZWQ", "Featured", query);
    add_department(departments, "GCUGFpZCBDaGFubmVscw", "Paid Channels", query);
    add_department(departments, "GCTXVzaWM", "Music", query);
    add_department(departments, "GCQ29tZWR5", "Comedy", query);
    EXPECT_CALL(reply, register_departments(IsDepartment(departments))).Times(
            1);

    expect_category(reply, renderer, "youtube", "Channels");

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "channel:UCdI8evszfZvyAl2UVCypkTA")),
            ResultProp("title", "MileyCyrusVEVO"),
            ResultProp("kind", "youtube#channel"),
            ResultProp("art", "https://yt3.ggpht.com/-7q31n1lfPcw/AAAAAAAAAAI/AAAAAAAAAAA/6otE9_5kJWc/s88-c-k-no/photo.jpg"),
            ResultProp("subtitle", "6590773 subscribers")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "channel:UC_TVqp_SyG6j5hG-xVRy95A")),
            ResultProp("title", "Skrillex"),
            ResultProp("kind", "youtube#channel"),
            ResultProp("art", "https://yt3.ggpht.com/-vE_ouJCWMQk/AAAAAAAAAAI/AAAAAAAAAAA/6bkr0eMOQ7o/s88-c-k-no/photo.jpg"),
            ResultProp("subtitle", "8162024 subscribers")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "channel:UC20vb-R_px4CguHzzBPhoyQ")),
            ResultProp("title", "EminemVEVO"),
            ResultProp("kind", "youtube#channel"),
            ResultProp("art", "https://yt3.ggpht.com/-NzI5Ni67ppc/AAAAAAAAAAI/AAAAAAAAAAA/7wGQowTOWWg/s88-c-k-no/photo.jpg"),
            ResultProp("subtitle", "12765775 subscribers")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "channel:UCpDJl2EmP7Oh90Vylx0dZtA")),
            ResultProp("title", "Spinnin' Records"),
            ResultProp("kind", "youtube#channel"),
            ResultProp("art", "https://yt3.ggpht.com/-yZkhExtYPZg/AAAAAAAAAAI/AAAAAAAAAAA/OfongtErwyo/s88-c-k-no/photo.jpg"),
            ResultProp("subtitle", "5963201 subscribers")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "channel:UCrDkAvwZum-UTjHmzDI2iIw")),
            ResultProp("title", "officialpsy"),
            ResultProp("kind", "youtube#channel"),
            ResultProp("art", "https://yt3.ggpht.com/-0Xgl841SU7Y/AAAAAAAAAAI/AAAAAAAAAAA/_bKTxRDm1kw/s88-c-k-no/photo.jpg"),
            ResultProp("subtitle", "7287121 subscribers")
        )))).WillOnce(Return(true));

    expect_category(reply, renderer, "youtube_login_nag", "");
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(
        ResultProp("title", "Log-in to YouTube")
        ))).WillOnce(
    Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_EN", "phone");
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(reply_proxy);
}

TEST_F(TestYoutubeScope, pick_department_videos) {
    const sc::CategoryRenderer renderer;
    NaggyMock<sct::MockSearchReply> reply;

    sc::CannedQuery query(SCOPE_NAME, "", "guideCategory-videos:GCTXVzaWM"); // pick the music videos department

    sc::Department::SPtr departments = sc::Department::create("", query,
            "Best of YouTube");
    add_department(departments, "GCRmVhdHVyZWQ", "Featured", query);
    add_department(departments, "GCUGFpZCBDaGFubmVscw", "Paid Channels", query);
    add_department(departments, "GCTXVzaWM", "Music", query);
    add_department(departments, "GCQ29tZWR5", "Comedy", query);
    EXPECT_CALL(reply, register_departments(IsDepartment(departments))).Times(
            1);

    expect_category(reply, renderer, "youtube", "Videos");

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "My2FRPA3Gf8"),
            ResultProp("title", "Miley Cyrus - Wrecking Ball"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/My2FRPA3Gf8/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "LrUvu1mlWco"),
            ResultProp("title", "Miley Cyrus - We Can't Stop"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/LrUvu1mlWco/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "sjSG6z_13-Q"),
            ResultProp("title", "Miley Cyrus - Can't Be Tamed"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/sjSG6z_13-Q/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "iVbQxC2c3-8"),
            ResultProp("title", "Miley Cyrus - Who Owns My Heart"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/iVbQxC2c3-8/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "8wxOVn99FTE"),
            ResultProp("title", "Miley Cyrus - When I Look At You"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/8wxOVn99FTE/hqdefault.jpg"),
            ResultProp("subtitle", "MileyCyrusVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "YJVmu6yttiw"),
            ResultProp("title", "SKRILLEX - Bangarang feat. Sirah [Official Music Video]"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/YJVmu6yttiw/hqdefault.jpg"),
            ResultProp("subtitle", "TheOfficialSkrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "2cXDgFwE13g"),
            ResultProp("title", "First Of The Year (Equinox) - Skrillex [OFFICIAL]"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/2cXDgFwE13g/hqdefault.jpg"),
            ResultProp("subtitle", "TheOfficialSkrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "WSeNSzJ2-Jw"),
            ResultProp("title", "SKRILLEX - Scary Monsters And Nice Sprites"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/WSeNSzJ2-Jw/hqdefault.jpg"),
            ResultProp("subtitle", "TheOfficialSkrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "BGpzGu9Yp6Y"),
            ResultProp("title", "Skrillex & Damian \"Jr. Gong\" Marley - Make It Bun Dem [OFFICIAL VIDEO]"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/BGpzGu9Yp6Y/hqdefault.jpg"),
            ResultProp("subtitle", "TheOfficialSkrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "eOofWzI3flA"),
            ResultProp("title", "Skrillex - Rock n Roll (Will Take You to the Mountain)"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/eOofWzI3flA/hqdefault.jpg"),
            ResultProp("subtitle", "TheOfficialSkrillex")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "uelHwf8o7_U"),
            ResultProp("title", "Eminem - Love The Way You Lie ft. Rihanna"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/uelHwf8o7_U/hqdefault.jpg"),
            ResultProp("subtitle", "EminemVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "j5-yKhDd64s"),
            ResultProp("title", "Eminem - Not Afraid"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/j5-yKhDd64s/hqdefault.jpg"),
            ResultProp("subtitle", "EminemVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "1wYNFfgrXTI"),
            ResultProp("title", "Eminem - When I'm Gone"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/1wYNFfgrXTI/hqdefault.jpg"),
            ResultProp("subtitle", "EminemVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "lgT1AidzRWM"),
            ResultProp("title", "Eminem - Beautiful"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/lgT1AidzRWM/hqdefault.jpg"),
            ResultProp("subtitle", "EminemVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "EHkozMIXZ8w"),
            ResultProp("title", "Eminem - The Monster (Explicit) ft. Rihanna"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/EHkozMIXZ8w/hqdefault.jpg"),
            ResultProp("subtitle", "EminemVEVO")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "gCYcHz2k5x0"),
            ResultProp("title", "Martin Garrix - Animals (Official Video)"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/gCYcHz2k5x0/hqdefault.jpg"),
            ResultProp("subtitle", "SpinninRec")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "p-Z3YrHJ1sU"),
            ResultProp("title", "Edward Maya & Vika Jigulina - Stereo Love (Official Music Video)"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/p-Z3YrHJ1sU/hqdefault.jpg"),
            ResultProp("subtitle", "SpinninRec")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "0EWbonj7f18"),
            ResultProp("title", "DVBBS & Borgeous - TSUNAMI (Original Mix)"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/0EWbonj7f18/hqdefault.jpg"),
            ResultProp("subtitle", "SpinninRec")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "uu_zwdmz0hE"),
            ResultProp("title", "Duck Sauce - Barbra Streisand (Official Music Video)"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/uu_zwdmz0hE/hqdefault.jpg"),
            ResultProp("subtitle", "SpinninRec")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "KnL2RJZTdA4"),
            ResultProp("title", "Martin Garrix & Jay Hardway - Wizard (Official Music Video) [OUT NOW]"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/KnL2RJZTdA4/hqdefault.jpg"),
            ResultProp("subtitle", "SpinninRec")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "9bZkp7q19f0"),
            ResultProp("title", "PSY - GANGNAM STYLE (강남스타일) M/V"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/9bZkp7q19f0/hqdefault.jpg"),
            ResultProp("subtitle", "officialpsy")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "ASO_zypdnsQ"),
            ResultProp("title", "PSY - GENTLEMAN M/V"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/ASO_zypdnsQ/hqdefault.jpg"),
            ResultProp("subtitle", "officialpsy")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "wcLNteez3c4"),
            ResultProp("title", "PSY (ft. HYUNA) 오빤 딱 내 스타일"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/wcLNteez3c4/hqdefault.jpg"),
            ResultProp("subtitle", "officialpsy")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "rX372ZwXOEM"),
            ResultProp("title", "PSY - GANGNAM STYLE @ Summer Stand Live Concert"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/rX372ZwXOEM/hqdefault.jpg"),
            ResultProp("subtitle", "officialpsy")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultProp("uri", "HkMNOlYcpHg"),
            ResultProp("title", "PSY - HANGOVER feat. Snoop Dogg M/V"),
            ResultProp("kind", "youtube#video"),
            ResultProp("art", "https://i.ytimg.com/vi/HkMNOlYcpHg/hqdefault.jpg"),
            ResultProp("subtitle", "officialpsy")
        )))).WillOnce(Return(true));

    expect_category(reply, renderer, "youtube_login_nag", "");
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(
        ResultProp("title", "Log-in to YouTube")
        ))).WillOnce(
    Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_EN", "phone");
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(reply_proxy);
}

TEST_F(TestYoutubeScope, pick_department_playlists) {
    const sc::CategoryRenderer renderer;
    NaggyMock<sct::MockSearchReply> reply;

    sc::CannedQuery query(SCOPE_NAME, "", "guideCategory-playlists:GCTXVzaWM"); // pick the music playlists department

    sc::Department::SPtr departments = sc::Department::create("", query,
            "Best of YouTube");
    add_department(departments, "GCRmVhdHVyZWQ", "Featured", query);
    add_department(departments, "GCUGFpZCBDaGFubmVscw", "Paid Channels", query);
    add_department(departments, "GCTXVzaWM", "Music", query);
    add_department(departments, "GCQ29tZWR5", "Comedy", query);
    EXPECT_CALL(reply, register_departments(IsDepartment(departments))).Times(
            1);

    expect_category(reply, renderer, "youtube", "Playlists");

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLR4XuJ-iybKvZWIlSwxp7KdBCjdBO5SDY")),
            ResultProp("title", "VEVO HQ Pop Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/9u3y5fmoAvA/default.jpg"),
            ResultProp("subtitle", "10 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLR4XuJ-iybKupktnWDLBQfJjdo7cHjOe4")),
            ResultProp("title", "VEVO HQ Pop Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/OJGUbwVMBeA/default.jpg"),
            ResultProp("subtitle", "10 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLR4XuJ-iybKvPSwG_cY6zaUoYmkgfIUw7")),
            ResultProp("title", "VEVO HQ Pop Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/IXpxe9xL-sk/default.jpg"),
            ResultProp("subtitle", "10 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLR4XuJ-iybKsKVOfO4p_di3Tg6Img_EgD")),
            ResultProp("title", "VEVO HQ Pop Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/H8tS5UQmNQM/default.jpg"),
            ResultProp("subtitle", "10 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLR4XuJ-iybKs9KYUKKvFSn-LLhCOp5qzO")),
            ResultProp("title", "VEVO HQ Pop Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/JJr80jXCepc/default.jpg"),
            ResultProp("subtitle", "10 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PL9Z0stL3aRykWNoVQW96JFIkelka_93Sc")),
            ResultProp("title", "RECESS"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/6JYIGclVQdw/default.jpg"),
            ResultProp("subtitle", "11 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PL9Z0stL3aRyk3-72RVYwMbYR3HbcK_U5r")),
            ResultProp("title", "TOUR VIDEOS"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i.ytimg.com/vi/default.jpg"),
            ResultProp("subtitle", "0 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PL9Z0stL3aRylHuDbVpSYJufB1VKc5RdWh")),
            ResultProp("title", "MUSIC VIDEOS"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/eOofWzI3flA/default.jpg"),
            ResultProp("subtitle", "8 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PL9Z0stL3aRynyGRTskjIQyTp7GI5oGXqo")),
            ResultProp("title", "POTATO"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/nuy1Gg_5AA0/default.jpg"),
            ResultProp("subtitle", "12 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PL9Z0stL3aRyk6yI9R2ja0oZKjOlqwQ-Ov")),
            ResultProp("title", "THE LEAVING EP"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/PoTp-TaOf_0/default.jpg"),
            ResultProp("subtitle", "3 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLRgSHCeagEV6Ptvt3E5gYNIZ6uqZvZ-tA")),
            ResultProp("title", "VEVO HQ Urban Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/9g91GUt2dVA/default.jpg"),
            ResultProp("subtitle", "6 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLRgSHCeagEV6OTbI31Zmdu0y6Tv4AM7m2")),
            ResultProp("title", "VEVO HQ Urban Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/XtlY1Da0jt4/default.jpg"),
            ResultProp("subtitle", "5 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLRgSHCeagEV782tXRkS5yRXBRIWena3Sc")),
            ResultProp("title", "VEVO HQ Urban Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/qL2DzPMFmdo/default.jpg"),
            ResultProp("subtitle", "6 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLRgSHCeagEV66rCmR8M4Sd7ytuJcsnXma")),
            ResultProp("title", "VEVO Urban Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/uh2DXqRRyXQ/default.jpg"),
            ResultProp("subtitle", "5 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLRgSHCeagEV5Gx6_PbU2nL41-Uwiz7eja")),
            ResultProp("title", "VEVO Urban Mix"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/USJZMrY4XqI/default.jpg"),
            ResultProp("subtitle", "5 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLx_tr69QV8CA2m08FmWj0jrPWQ1T3Buul")),
            ResultProp("title", "Barong Family"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/bSZFMOO-U0o/default.jpg"),
            ResultProp("subtitle", "1 video")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLx_tr69QV8CB5nZduKd8SCs_ttLWqN27N")),
            ResultProp("title", "Flye Eye Records"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/sjmswPj372w/default.jpg"),
            ResultProp("subtitle", "3 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLx_tr69QV8CD3sTHkAaYlzNQPdjxr8g62")),
            ResultProp("title", "Ultra Music Festival 2014 Live Sets"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/mDLcj6y9eko/default.jpg"),
            ResultProp("subtitle", "9 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLx_tr69QV8CAbkVXzP7fjz0fmalAlhAN2")),
            ResultProp("title", "Skink"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/eTN9dPAmZFQ/default.jpg"),
            ResultProp("subtitle", "3 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLx_tr69QV8CCyUSdcd9xqYwW_n-M6n6za")),
            ResultProp("title", "SPRS"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/03DabSVmHVs/default.jpg"),
            ResultProp("subtitle", "34 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLu8-5UhSJGkJRAlpnB8xlw7Mf78yFGHYG")),
            ResultProp("title", "YG Family Featured Playlists"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/7LP4foN3Xs4/default.jpg"),
            ResultProp("subtitle", "55 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PLEC422D53B7588DC7")),
            ResultProp("title", "PSY Featured Playlists"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/9bZkp7q19f0/default.jpg"),
            ResultProp("subtitle", "28 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:PL950C8AEC6CC3E6FE")),
            ResultProp("title", "Music Videos"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/1cKc1rkZwf8/default.jpg"),
            ResultProp("subtitle", "9 videos")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
            ResultUriMatchesCannedQuery(sc::CannedQuery(SCOPE_INSTALL_NAME, "", "playlist:FLrDkAvwZum-UTjHmzDI2iIw")),
            ResultProp("title", "Favorites"),
            ResultProp("kind", "youtube#playlist"),
            ResultProp("art", "https://i1.ytimg.com/vi/hNbi9rZaVOA/default.jpg"),
            ResultProp("subtitle", "42 videos")
        )))).WillOnce(Return(true));

    expect_category(reply, renderer, "youtube_login_nag", "");
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(
        ResultProp("title", "Log-in to YouTube")
        ))).WillOnce(
    Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_EN", "phone");
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);
    search_query->run(reply_proxy);
}

} // namespace
