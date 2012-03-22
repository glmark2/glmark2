/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010-2012 Linaro Limited
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
 *  Jesse Barker (glmark2)
 */
#include "gl-headers.h"
#include "scene.h"
#include "benchmark.h"
#include "options.h"
#include "log.h"
#include "util.h"
#include "text-renderer.h"
#include "main-loop.h"
#include "benchmark-collection.h"

#include <iostream>
#include <fstream>

#if USE_GL
#include "canvas-x11-glx.h"
#elif USE_GLESv2
#include "canvas-x11-egl.h"
#endif

using std::vector;
using std::map;
using std::string;

void
add_and_register_scenes(vector<Scene*>& scenes, Canvas& canvas)
{
    scenes.push_back(new SceneDefaultOptions(canvas));
    scenes.push_back(new SceneBuild(canvas));
    scenes.push_back(new SceneTexture(canvas));
    scenes.push_back(new SceneShading(canvas));
    scenes.push_back(new SceneConditionals(canvas));
    scenes.push_back(new SceneFunction(canvas));
    scenes.push_back(new SceneLoop(canvas));
    scenes.push_back(new SceneBump(canvas));
    scenes.push_back(new SceneEffect2D(canvas));
    scenes.push_back(new ScenePulsar(canvas));
    scenes.push_back(new SceneDesktop(canvas));
    scenes.push_back(new SceneBuffer(canvas));

    for (vector<Scene*>::const_iterator iter = scenes.begin();
         iter != scenes.end();
         iter++)
    {
        Benchmark::register_scene(**iter);
    }
}

static void
list_scenes()
{
    const map<string, Scene *> &scenes = Benchmark::scenes();

    for (map<string, Scene *>::const_iterator scene_iter = scenes.begin();
         scene_iter != scenes.end();
         scene_iter++)
    {
        Scene *scene = scene_iter->second;
        if (scene->name().empty())
            continue;
        Log::info("[Scene] %s\n", scene->name().c_str());

        const map<string, Scene::Option> &options = scene->options();

        for (map<string, Scene::Option>::const_iterator opt_iter = options.begin();
             opt_iter != options.end();
             opt_iter++)
        {
            const Scene::Option &opt = opt_iter->second;
            Log::info("  [Option] %s\n"
                      "    Description  : %s\n"
                      "    Default Value: %s\n",
                      opt.name.c_str(),
                      opt.description.c_str(),
                      opt.default_value.c_str());
        }
    }
}

void
do_benchmark(Canvas &canvas)
{
    BenchmarkCollection benchmark_collection;
    MainLoop *loop;

    benchmark_collection.populate_from_options();
    
    if (benchmark_collection.needs_decoration())
        loop = new MainLoopDecoration(canvas, benchmark_collection.benchmarks());
    else
        loop = new MainLoop(canvas, benchmark_collection.benchmarks());

    while (loop->step());

    Log::info("=======================================================\n");
    Log::info("                                  glmark2 Score: %u \n", loop->score());
    Log::info("=======================================================\n");

    delete loop;
}

void
do_validation(Canvas &canvas)
{
    BenchmarkCollection benchmark_collection;

    benchmark_collection.populate_from_options();

    MainLoopValidation loop(canvas, benchmark_collection.benchmarks());

    while (loop.step());
}

int
main(int argc, char *argv[])
{

    if (!Options::parse_args(argc, argv))
        return 1;

    /* Initialize Log class */
    Log::init(Util::appname_from_path(argv[0]), Options::show_debug);

    if (Options::show_help) {
        Options::print_help();
        return 0;
    }

    /* Force 800x600 output for validation */
    if (Options::validate &&
        Options::size != std::pair<int,int>(800, 600))
    {
        Log::info("Ignoring custom size %dx%d for validation. Using 800x600.\n",
                  Options::size.first, Options::size.second);
        Options::size = std::pair<int,int>(800, 600);
    }

    // Create the canvas
#if USE_GL
    CanvasX11GLX canvas(Options::size.first, Options::size.second);
#elif USE_GLESv2
    CanvasX11EGL canvas(Options::size.first, Options::size.second);
#endif

    canvas.offscreen(Options::offscreen);

    vector<Scene*> scenes;

    // Register the scenes, so they can be looked up by name
    add_and_register_scenes(scenes, canvas);

    if (Options::list_scenes) {
        list_scenes();
        return 0;
    }

    if (!canvas.init()) {
        Log::error("%s: Could not initialize canvas\n", __FUNCTION__);
        return 1;
    }

    Log::info("=======================================================\n");
    Log::info("    glmark2 %s\n", GLMARK_VERSION);
    Log::info("=======================================================\n");
    canvas.print_info();
    Log::info("=======================================================\n");

    canvas.visible(true);

    if (Options::validate)
        do_validation(canvas);
    else
        do_benchmark(canvas);

    Util::dispose_pointer_vector(scenes);

    return 0;
}
