//
// Copyright (c) 2010-2011 Linaro Limited
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License which accompanies
// this distribution, and is available at
// http://www.opensource.org/licenses/mit-license.php
//
// Contributors:
//     Alexandros Frantzis <alexandros.frantzis@linaro.org>
//     Jesse Barker <jesse.barker@linaro.org>
//
#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>
#include <istream>
#include <sstream>
#include <stdint.h>

#ifdef ANDROID
#include <android/asset_manager_jni.h>
#endif

struct Util {
    static void split(const std::string &s, char delim, std::vector<std::string> &elems);
    static uint64_t get_timestamp_us();
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
    static std::string
    appname_from_path(const std::string& path);

#ifdef ANDROID
    static void android_set_asset_manager(AAssetManager *asset_manager);
    static AAssetManager *android_get_asset_manager(void);
private:
    static AAssetManager *android_asset_manager;
#endif
};

#endif /* UTIL_H */
