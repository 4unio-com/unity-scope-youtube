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

#ifndef YOUTUBE_API_SEARCHLISTRESPONSE_H_
#define YOUTUBE_API_SEARCHLISTRESPONSE_H_

#include <youtube/api/resource.h>

#include <deque>
#include <memory>

namespace Json {
class Value;
}

namespace youtube {
namespace api {

class SearchListResponse {
public:
    typedef std::shared_ptr<SearchListResponse> Ptr;

    typedef std::deque<Resource::Ptr> ResourceList;

    SearchListResponse(const Json::Value &data);

    ~SearchListResponse() = default;

    ResourceList items();

    std::size_t total_results() const;

protected:
    ResourceList items_;

    std::size_t total_results_;
};

}
}

#endif /* YOUTUBE_API_SEARCHLISTRESPONSE_H_ */
