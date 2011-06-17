/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010-2011 Linaro Limited
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
 *  Ben Smith (original glmark benchmark)
 *  Alexandros Frantzis (glmark2)
 */
#include "scene.h"
#include "log.h"
#include <sstream>
#include <cmath>

using std::stringstream;
using std::string;
using std::map;

Scene::Scene(Screen &pScreen, const string &name) :
    mScreen(pScreen), mName(name),
    mStartTime(0), mLastUpdateTime(0), mCurrentFrame(0), mAverageFPS(0), 
    mRunning(0), mDuration(0)
{
    mOptions["duration"] = Scene::Option("duration", "10.0",
                                         "The duration of each benchmark in seconds");
}

Scene::~Scene()
{
}

int Scene::load()
{
    return 1;
}

void Scene::unload()
{
}

void Scene::setup()
{
    stringstream ss(mOptions["duration"].value);
    ss >> mDuration;
}

void Scene::teardown()
{
}

void Scene::update()
{
}

void Scene::draw()
{
}

string
Scene::info_string(const string &title)
{
    stringstream ss;

    ss << "[" << mName << "] " << Scene::construct_title(title) << " ";

    return ss.str();
}

unsigned Scene::average_fps()
{
    return mAverageFPS;
}


bool Scene::is_running()
{
    return mRunning;
}

bool
Scene::set_option(const string &opt, const string &val)
{ 
    map<string, Option>::iterator iter = mOptions.find(opt);

    if (iter == mOptions.end())
        return false;

    iter->second.value = val;

    return true;
}

void
Scene::reset_options()
{
    for (map<string, Option>::iterator iter = mOptions.begin();
         iter != mOptions.end();
         iter++)
    {
        Option &opt = iter->second;

        opt.value = opt.default_value;
    }
}


string
Scene::construct_title(const string &title)
{
    stringstream ss;

    if (title == "") {
        for (map<string, Option>::iterator iter = mOptions.begin();
             iter != mOptions.end();
             iter++)
        {
            ss << iter->first << "=" << iter->second.value << ":";
        }
    }
    else
        ss << title;

    return ss.str();

}

double
Scene::pixel_value_distance(Screen::Pixel p1, Screen::Pixel p2,
                            bool use_alpha)
{
    double s(0.0);

    // These work without casts because of integer promotion rules
    // (the Uint8s are promoted to ints)
    s += (p1.r - p2.r) * (p1.r - p2.r);
    s += (p1.g - p2.g) * (p1.g - p2.g);
    s += (p1.b - p2.b) * (p1.b - p2.b);

    if (use_alpha)
        s += (p1.a - p2.a) * (p1.a - p2.a);

    return std::sqrt(s);
}

bool
Scene::load_shaders(Program &program,
                    const std::string &vtx_shader_filename,
                    const std::string &frg_shader_filename)
{
    std::string vtx_shader;
    std::string frg_shader;

    if (!gotSource(vtx_shader_filename, vtx_shader))
        return false;

    if (!gotSource(frg_shader_filename, frg_shader))
        return false;

    program.init();

    program.addShader(GL_VERTEX_SHADER, vtx_shader);
    if (!program.valid()) {
        Log::error("Failed to add vertex shader from file %s:\n  %s\n",
                   vtx_shader_filename.c_str(),
                   program.errorMessage().c_str());
        program.release();
        return false;
    }

    program.addShader(GL_FRAGMENT_SHADER, frg_shader);
    if (!program.valid()) {
        Log::error("Failed to add fragment shader from file %s:\n  %s\n",
                   frg_shader_filename.c_str(),
                   program.errorMessage().c_str());
        program.release();
        return false;
    }

    program.build();
    if (!program.ready()) {
        Log::error("Failed to link program created from files %s and %s:  %s\n",
                   vtx_shader_filename.c_str(),
                   frg_shader_filename.c_str(),
                   program.errorMessage().c_str());
        program.release();
        return false;
    }

    return true;
}
