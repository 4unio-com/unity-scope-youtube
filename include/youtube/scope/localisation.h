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

#ifndef YOUTUBE_SCOPE_LOCALISATION_H_
#define YOUTUBE_SCOPE_LOCALISATION_H_

#include <libintl.h>

#include <boost/format.hpp>

inline char * _(const char *__msgid) {
    return dgettext(GETTEXT_PACKAGE, __msgid);
}

inline std::string _(const char *__msgid1, const char *__msgid2,
        unsigned long int __n) {
    boost::format fmt(dngettext(GETTEXT_PACKAGE, __msgid1, __msgid2, __n));
    return boost::str(fmt % __n);
}

#endif // YOUTUBE_SCOPE_LOCALISATION_H_
