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
#include "shader-source.h"
#include "options.h"
#include "util.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/resource.h>
#include <unistd.h>

using std::stringstream;
using std::string;
using std::map;

double Scene::shaderCompilationTime_ = 0.0;

Scene::Option::Option(const std::string &nam, const std::string &val, const std::string &desc,
                      const std::string &values) :
name(nam), value(val), default_value(val), description(desc), set(false)
{
    Util::split(values, ',', acceptable_values, Util::SplitModeNormal);
}

Scene::Scene(Canvas &pCanvas, const string &name) :
    canvas_(pCanvas), name_(name),
    currentFrame_(0), running_(0), duration_(0), nframes_(0)
{
    options_["duration"] = Scene::Option("duration", "10.0",
                                         "The duration of each benchmark in seconds");
    options_["nframes"] = Scene::Option("nframes", "",
                                         "The number of frames to render");
    options_["vertex-precision"] = Scene::Option("vertex-precision",
                                                 "default,default,default,default",
                                                 "The precision values for the vertex shader (\"int,float,sampler2d,samplercube\")");
    options_["fragment-precision"] = Scene::Option("fragment-precision",
                                                   "default,default,default,default",
                                                   "The precision values for the fragment shader (\"int,float,sampler2d,samplercube\")");
    /* FPS options */
    options_["show-fps"] = Scene::Option("show-fps", "false",
                                         "Show live FPS counter",
                                         "false,true");
    options_["fps-pos"] = Scene::Option("fps-pos", "-1.0,-1.0",
                                         "The position on screen where to show FPS");
    options_["fps-size"] = Scene::Option("fps-size", "0.03",
                                         "The width of each glyph for the FPS");
    /* Title options */
    options_["title"] = Scene::Option("title", "",
                                      "The scene title to show");
    options_["title-pos"] = Scene::Option("title-pos", "-0.7,-1.0",
                                      "The position on screen where to show the title");
    options_["title-size"] = Scene::Option("title-size", "0.03",
                                           "The width of each glyph in the title");
}

Scene::~Scene()
{
}

bool
Scene::supported(bool show_errors)
{
    static_cast<void>(show_errors);
    return true;
}

bool
Scene::load()
{
    return true;
}

void
Scene::unload()
{
}

bool
Scene::setup()
{
    return true;
}

void
Scene::teardown()
{
}

void
Scene::update()
{
    realTime_.lastUpdate = Util::get_timestamp_us() / 1000000.0;

    currentFrame_++;

    if (realTime_.elapsed() >= duration_)
        running_ = false;

    if (nframes_ > 0 && currentFrame_ >= nframes_)
        running_ = false;
}

void
Scene::draw()
{
}

string
Scene::info_string(const string &title)
{
    stringstream ss;

    ss << "[" << name_ << "] " << Scene::construct_title(title);

    return ss.str();
}

unsigned
Scene::average_fps()
{
    return currentFrame_ / realTime_.elapsed();
}

Scene::Stats
Scene::stats()
{
    Stats stats;
    int nproc = sysconf(_SC_NPROCESSORS_ONLN);

    stats.average_frame_time = realTime_.elapsed() / currentFrame_;
    stats.average_user_time = userTime_.elapsed() / currentFrame_;
    stats.average_system_time = systemTime_.elapsed() / currentFrame_;
    if (idleTime_.lastUpdate == 0.0)
        stats.cpu_busy_percent = 0.0;
    else
        stats.cpu_busy_percent = 1.0 - idleTime_.elapsed() /
                                       (nproc * realTime_.elapsed());
    stats.shader_compilation_time = shaderCompilationTime_;

    return stats;
}

bool
Scene::set_option(const string &opt, const string &val)
{
    map<string, Option>::iterator iter = options_.find(opt);

    if (iter == options_.end())
        return false;

    std::vector<std::string> &values(iter->second.acceptable_values);

    if (!values.empty() && 
        std::find(values.begin(), values.end(), val) == values.end())
    {
            return false;
    }

    iter->second.value = val;
    iter->second.set = true;

    return true;
}

void
Scene::reset_options()
{
    for (map<string, Option>::iterator iter = options_.begin();
         iter != options_.end();
         iter++)
    {
        Option &opt = iter->second;

        opt.value = opt.default_value;
        opt.set = false;
    }
}

bool
Scene::set_option_default(const string &opt, const string &val)
{
    map<string, Option>::iterator iter = options_.find(opt);

    if (iter == options_.end())
        return false;

    std::vector<std::string> &values(iter->second.acceptable_values);

    if (!values.empty() && 
        std::find(values.begin(), values.end(), val) == values.end())
    {
            return false;
    }

    iter->second.default_value = val;

    return true;
}

bool
Scene::prepare()
{
    duration_ = Util::fromString<double>(options_["duration"].value);
    nframes_ = Util::fromString<unsigned>(options_["nframes"].value);

    ShaderSource::default_precision(
            ShaderSource::Precision(options_["vertex-precision"].value),
            ShaderSource::ShaderTypeVertex
            );

    ShaderSource::default_precision(
            ShaderSource::Precision(options_["fragment-precision"].value),
            ShaderSource::ShaderTypeFragment
            );

    currentFrame_ = 0;
    running_ = false;
    realTime_.start = realTime_.lastUpdate = 0;
    userTime_.start = userTime_.lastUpdate = 0;
    systemTime_.start = systemTime_.lastUpdate = 0;
    shaderCompilationTime_ = 0.0;

    if (!supported(true))
        return false;

    if (!load() || !setup())
        return false;

    running_ = true;
    update_elapsed_times();
    realTime_.start = realTime_.lastUpdate;
    userTime_.start = userTime_.lastUpdate;
    systemTime_.start = systemTime_.lastUpdate;
    idleTime_.start = idleTime_.lastUpdate;

    return true;
}

void
Scene::finish()
{
    update_elapsed_times();
    teardown();
    unload();
}

string
Scene::construct_title(const string &title)
{
    stringstream ss;
    bool first_option = true;

    if (title == "") {
        for (map<string, Option>::iterator iter = options_.begin();
             iter != options_.end();
             iter++)
        {
            if (Options::show_all_options || iter->second.set)
            {
                if (first_option)
                    first_option = false;
                else
                    ss << ":";
                ss << iter->first << "=" << iter->second.value;
            }
        }

        if (ss.str().empty())
            ss << "<default>";
    }
    else
        ss << title;

    return ss.str();

}

bool
Scene::load_shaders_from_strings(Program &program,
                                 const std::string &vtx_shader,
                                 const std::string &frg_shader,
                                 const std::string &vtx_shader_filename,
                                 const std::string &frg_shader_filename)
{
    double shaderStartTime = Util::get_timestamp_us() / 1000000.0;

    program.init();

    Log::debug("Loading vertex shader from file %s:\n%s",
               vtx_shader_filename.c_str(), vtx_shader.c_str());

    program.addShader(GL_VERTEX_SHADER, vtx_shader);
    if (!program.valid()) {
        Log::error("Failed to add vertex shader from file %s:\n  %s\n",
                   vtx_shader_filename.c_str(),
                   program.errorMessage().c_str());
        program.release();
        return false;
    }

    Log::debug("Loading fragment shader from file %s:\n%s",
               frg_shader_filename.c_str(), frg_shader.c_str());

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

    shaderCompilationTime_ += Util::get_timestamp_us() / 1000000.0 - shaderStartTime;

    return true;
}

void
Scene::update_elapsed_times()
{
    realTime_.lastUpdate = Util::get_timestamp_us() / 1000000.0;

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    userTime_.lastUpdate = usage.ru_utime.tv_sec +
                           usage.ru_utime.tv_usec / 1000000.0;
    systemTime_.lastUpdate = usage.ru_stime.tv_sec +
                             usage.ru_stime.tv_usec / 1000000.0;

    double uptime, idle;
    std::ifstream ifs("/proc/uptime");
    ifs >> uptime >> idle;
    if (!ifs.fail())
        idleTime_.lastUpdate = idle;
}
