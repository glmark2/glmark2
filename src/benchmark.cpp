/*
 * Copyright © 2011 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis (glmark2)
 */

#include "benchmark.h"
#include "log.h"
#include "util.h"

using std::string;
using std::vector;
using std::map;

std::map<string, Scene *> Benchmark::mSceneMap;

static Scene &
get_scene_from_description(const string &s)
{
    vector<string> elems;

    Util::split(s, ':', elems);

    const string &name = !elems.empty() ? elems[0] : ""; 

    return Benchmark::get_scene_by_name(name);
}

static vector<Benchmark::OptionPair>
get_options_from_description(const string &s)
{
    vector<Benchmark::OptionPair> options;
    vector<string> elems;

    Util::split(s, ':', elems);

    for (vector<string>::const_iterator iter = ++elems.begin();
         iter != elems.end();
         iter++)
    {
        vector<string> opt;

        Util::split(*iter, '=', opt);
        if (opt.size() == 2)
            options.push_back(Benchmark::OptionPair(opt[0], opt[1]));
        else
            Log::info("Warning: ignoring invalid option string '%s' "
                      "in benchmark description\n",
                      iter->c_str());
    }

    return options;
}

void
Benchmark::register_scene(Scene &scene)
{
    mSceneMap[scene.name()] = &scene;
}

Scene &
Benchmark::get_scene_by_name(const string &name)
{
    map<string, Scene *>::const_iterator iter;

    if ((iter = mSceneMap.find(name)) != mSceneMap.end())
        return *(iter->second);
    else
        return Scene::dummy();
}

Benchmark::Benchmark(Scene &scene, const vector<OptionPair> &options) :
    mScene(scene), mOptions(options)
{
}

Benchmark::Benchmark(const string &name, const vector<OptionPair> &options) :
    mScene(Benchmark::get_scene_by_name(name)), mOptions(options)
{
}

Benchmark::Benchmark(const string &s) :
    mScene(get_scene_from_description(s)),
    mOptions(get_options_from_description(s))
{
}

Scene &
Benchmark::setup_scene()
{
    mScene.reset_options();
    load_options();

    mScene.load();
    mScene.setup();

    return mScene;
}

void
Benchmark::teardown_scene()
{
    mScene.teardown();
    mScene.unload();
}

void
Benchmark::load_options()
{
    for (vector<OptionPair>::iterator iter = mOptions.begin();
         iter != mOptions.end();
         iter++)
    {
        mScene.set_option(iter->first, iter->second);
    }
}

