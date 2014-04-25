/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 */

#ifndef YOUTUBEDOWNLOADER_H_
#define YOUTUBEDOWNLOADER_H_

#include<string>

class YoutubeDownloader final {
public:
    YoutubeDownloader(const std::string &apikey);
    ~YoutubeDownloader();
    YoutubeDownloader(const YoutubeDownloader &) = delete;
    YoutubeDownloader& operator=(const YoutubeDownloader &) = delete;

    void query(const std::string &q);

private:
    struct Private;
    Private *p;
};

#endif
