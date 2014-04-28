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

#include<iostream>
#include<fstream>
#include"YoutubeDownloader.h"

int main(int argc, char **argv) {
    std::ifstream apifile("apikey.txt", std::ios::in);
    if(apifile.bad()) {
        std::cout << "API file open is fail.\n";
        return 1;
    }
    std::string apikey;
    std::getline(apifile, apikey);
    if(apikey.empty()) {
        std::cout << "API file read is fail.\n";
        return 1;
    }
    YoutubeDownloader yd(apikey);
    auto results = yd.query("cats");
    for(const auto &i : results) {
        std::cout << i.title << std::endl;
        std::cout << " description: " << i.description << std::endl;
        std::cout << " screenshot:  " << i.screenshot << std::endl;
        std::cout << std::endl;
    }
    return 0;
}
