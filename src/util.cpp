/*
 * Copyright © 2011 Linaro Limited
 *
 * This file is part of glcompbench.
 *
 * glcompbench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glcompbench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glcompbench.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis <alexandros.frantzis@linaro.org>
 *  Jesse Barker <jesse.barker@linaro.org>
 */

#include <sstream>
#include <fstream>
#ifdef ANDROID
#include <android/asset_manager.h>
#endif

#include "log.h"
#include "util.h"

/** 
 * Splits a string using a delimiter
 * 
 * @param s the string to split
 * @param delim the delimitir to use
 * @param elems the string vector to populate
 */
void
Util::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);

    std::string item;
    while(std::getline(ss, item, delim))
        elems.push_back(item);
}

#ifndef ANDROID

std::istream *
Util::get_resource(const std::string &path)
{
    std::ifstream *ifs = new std::ifstream(path.c_str());

    return static_cast<std::istream *>(ifs);
}

#else

AAssetManager *Util::android_asset_manager = 0;

void
Util::android_set_asset_manager(AAssetManager *asset_manager)
{
    Util::android_asset_manager = asset_manager;
}

AAssetManager *
Util::android_get_asset_manager()
{
    return Util::android_asset_manager;
}

std::istream *
Util::get_resource(const std::string &path)
{
    std::string path2(path);
    /* Remove leading '/' */
    path2.erase(0, 1);

    std::stringstream *ss = new std::stringstream;
    AAsset *asset = AAssetManager_open(Util::android_asset_manager,
                                       path2.c_str(), AASSET_MODE_RANDOM);
    if (asset) {
        ss->write(reinterpret_cast<const char *>(AAsset_getBuffer(asset)),
                  AAsset_getLength(asset));
        Log::debug("Load asset %s\n", path2.c_str());
        AAsset_close(asset);
    }
    else {
        Log::error("Couldn't load asset %s\n", path2.c_str());
    }

    return static_cast<std::istream *>(ss);
}
#endif
