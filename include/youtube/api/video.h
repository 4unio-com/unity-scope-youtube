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

#ifndef YOUTUBE_API_VIDEO_H_
#define YOUTUBE_API_VIDEO_H_

#include <youtube/api/resource.h>

#include <memory>
#include <string>

namespace Json {
class Value;
}

namespace youtube {
namespace api {

class Video: public Resource {
public:
    typedef std::shared_ptr<Video> Ptr;

    struct Statistics {
        std::string comment_count;
        std::string dislike_count;
        std::string favorite_count;
        std::string like_count;
        std::string view_count;
    };

    Video(const Json::Value &data);

    virtual ~Video() = default;

    const std::string & title() const override;

    const std::string & username() const;

    const std::string & link() const;

    const std::string & picture() const override;

    const std::string & description() const;

    const std::string & id() const override;

    Kind kind() const override;

    std::string kind_str() const override;

    bool has_statistics() const;

    const Statistics & statistics() const;

protected:
    std::string title_;

    std::string username_;

    std::string id_;

    std::string link_;

    std::string picture_;

    std::string description_;

    bool has_statistics_;

    Statistics statistics_;
};

}
}

#endif // YOUTUBE_API_VIDEO_H_
