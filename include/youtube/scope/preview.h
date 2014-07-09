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

#ifndef YOUTUBE_SCOPE_PREVIEW_H_
#define YOUTUBE_SCOPE_PREVIEW_H_

#include <youtube/api/client.h>

#include <unity/scopes/PreviewQueryBase.h>

namespace unity {
namespace scopes {
class Result;
}
}

namespace youtube {
namespace scope {

class Preview: public unity::scopes::PreviewQueryBase {
public:
    Preview(const unity::scopes::Result &result,
            const unity::scopes::ActionMetadata &metadata,
            youtube::api::Client::Ptr client);

    ~Preview() = default;

    void cancelled() override;

    void run(unity::scopes::PreviewReplyProxy const& reply) override;

protected:
    void playable(const unity::scopes::PreviewReplyProxy& reply);

    void playlist(const unity::scopes::PreviewReplyProxy& reply);

    youtube::api::Client::Ptr client_;
};

}
}

#endif // YOUTUBE_SCOPE_PREVIEW_H_
