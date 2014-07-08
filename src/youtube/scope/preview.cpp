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

#include <youtube/scope/preview.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>

namespace sc = unity::scopes;

using namespace std;
using namespace youtube::scope;

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata) :
        sc::PreviewQueryBase(result, metadata) {
}

void Preview::cancelled() {
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);
    layout1col.add_column( { "video", "header", "summary", "actions" });

    layout2col.add_column( { "video" });
    layout2col.add_column( { "header", "summary", "actions" });

    layout3col.add_column( { "video" });
    layout3col.add_column( { "header", "summary" });
    layout3col.add_column( { "actions" });

    reply->register_layout( { layout1col, layout2col, layout3col });

    sc::PreviewWidget header("header", "header");
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "subtitle");

    sc::PreviewWidget video("video", "video");
    video.add_attribute_mapping("source", "link");
    video.add_attribute_mapping("screenshot", "art");

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

    reply->push( { video, header, description, actions });
}
