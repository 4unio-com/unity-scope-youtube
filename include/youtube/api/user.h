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
 *         Gary Wang  <gary.wang@canonical.com>
 */

#ifndef API_USER_H_
#define API_USER_H_

#include <youtube/api/resource.h>

#include <memory>
#include <string>

namespace Json {
class Value;
}

namespace youtube {
namespace api {

class User: public Resource {
public:
    typedef std::shared_ptr<User> Ptr;

    User(const Json::Value &data);

    virtual ~User() = default;

    const std::string & title() const override;

    const std::string & id() const override;

    const std::string & picture() const override;

    Kind kind() const override;

    std::string kind_str() const override;

protected:
    std::string title_;

    std::string id_;

    std::string picture_;
};

}
}

#endif // API_USER_H_
