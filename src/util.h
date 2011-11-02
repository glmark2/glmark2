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

#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>
#include <istream>
#include <sstream>

#ifdef ANDROID
#include <android/asset_manager_jni.h>
#endif

struct Util {
    static void split(const std::string &s, char delim, std::vector<std::string> &elems);
    static std::istream *get_resource(const std::string &path);
    static void list_files(const std::string& dirName, std::vector<std::string>& fileVec);
    template <class T> static void dispose_pointer_vector(std::vector<T*> &vec)
    {
        for (typename std::vector<T*>::const_iterator iter = vec.begin();
             iter != vec.end();
             iter++)
        {
            delete *iter;
        }

        vec.clear();
    }
    template<typename T>
    static T
    fromString(const std::string& asString)
    {
        std::stringstream ss(asString);
        T retVal;
        ss >> retVal;
        return retVal;
    }

    template<typename T>
    static std::string
    toString(const T t)
    {
        std::stringstream ss;
        ss << t;
        return ss.str();
    }


#ifdef ANDROID
    static void android_set_asset_manager(AAssetManager *asset_manager);
    static AAssetManager *android_get_asset_manager(void);
private:
    static AAssetManager *android_asset_manager;
#endif
};

#endif /* UTIL_H */
