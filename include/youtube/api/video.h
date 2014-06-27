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

#include <memory>
#include <string>

namespace Json {
class Value;
}

namespace youtube {
namespace api {

class Video {
public:
    typedef std::shared_ptr<Video> Ptr;

    Video(const Json::Value &data);

    virtual ~Video() = default;

    const std::string & name() const;

    const std::string & username() const;

    const std::string & uri() const;

    const std::string & picture() const;

    const std::string & description() const;

protected:
    std::string name_;

    std::string username_;

    std::string uri_;

    std::string picture_;

    std::string description_;
};

}
}

#endif // YOUTUBE_API_VIDEO_H_
