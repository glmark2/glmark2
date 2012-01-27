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
#include <sstream>
#include <fstream>
#include <sys/time.h>
#ifdef ANDROID
#include <android/asset_manager.h>
#else
#include <dirent.h>
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

uint64_t
Util::get_timestamp_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t now = static_cast<uint64_t>(tv.tv_sec) * 1000000 +
                   static_cast<double>(tv.tv_usec);
    return now;
}

std::string
Util::appname_from_path(const std::string& path)
{
    std::string::size_type slashPos = path.rfind("/");
    std::string::size_type startPos(0);
    if (slashPos != std::string::npos)
    {
        startPos = slashPos + 1;
    }
    return std::string(path, startPos, std::string::npos);
}

#ifndef ANDROID

std::istream *
Util::get_resource(const std::string &path)
{
    std::ifstream *ifs = new std::ifstream(path.c_str());

    return static_cast<std::istream *>(ifs);
}

void
Util::list_files(const std::string& dirName, std::vector<std::string>& fileVec)
{
    DIR* dir = opendir(dirName.c_str());
    if (!dir)
    {
        Log::error("Failed to open models directory '%s'\n", dirName.c_str());
        return;
    }

    struct dirent* entry = readdir(dir);
    while (entry)
    {
        std::string pathname(dirName + "/");
        pathname += std::string(entry->d_name);
        // Skip '.' and '..'
        if (entry->d_name[0] != '.')
        {
            fileVec.push_back(pathname);
        }
        entry = readdir(dir);
    }
    closedir(dir);
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
    /* Remove leading '/' from path name, it confuses the AssetManager */
    if (path2.size() > 0 && path2[0] == '/')
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

void
Util::list_files(const std::string& dirName, std::vector<std::string>& fileVec)
{
    AAssetManager *mgr(Util::android_get_asset_manager());
    std::string dir_name(dirName);

    /* Remove leading '/' from path, it confuses the AssetManager */
    if (dir_name.size() > 0 && dir_name[0] == '/')
        dir_name.erase(0, 1);

    AAssetDir* dir = AAssetManager_openDir(mgr, dir_name.c_str());
    if (!dir)
    {
        Log::error("Failed to open models directory '%s'\n", dir_name.c_str());
        return;
    }

    const char *filename(0);
    while ((filename = AAssetDir_getNextFileName(dir)) != 0)
    {
        std::string pathname(dir_name + "/");
        pathname += std::string(filename);
        fileVec.push_back(pathname);
    }
    AAssetDir_close(dir);
}
#endif
