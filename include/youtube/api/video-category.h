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

#ifndef YOUTUBE_API_VIDEOCATEGORY_H_
#define YOUTUBE_API_VIDEOCATEGORY_H_

#include <memory>

namespace Json {
class Value;
}

namespace youtube {
namespace api {

class VideoCategory {
public:
    typedef std::shared_ptr<VideoCategory> Ptr;

    VideoCategory(const Json::Value &data);

    ~VideoCategory() = default;

    const std::string & name() const;

    const std::string & id() const;

protected:
    std::string name_;

    std::string id_;

    std::string content_rating_;
};

}
}

#endif /* YOUTUBE_API_VIDEOCATEGORY_H_ */
