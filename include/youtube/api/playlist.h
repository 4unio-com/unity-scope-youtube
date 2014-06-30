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

#ifndef YOUTUBE_API_PLAYLIST_H_
#define YOUTUBE_API_PLAYLIST_H_

#include <youtube/api/resource.h>

#include <memory>

namespace Json {
class Value;
}

namespace youtube {
namespace api {

class Playlist: public Resource {
public:
    typedef std::shared_ptr<Playlist> Ptr;

    Playlist(const Json::Value &data);

    ~Playlist() = default;

    const std::string & title() const override;

    const std::string & picture() const override;

    const std::string & id() const override;

    Kind kind() const override;

protected:
    std::string name_;

    std::string picture_;

    std::string id_;
};

}
}

#endif /* YOUTUBE_API_PLAYLIST_H_ */
