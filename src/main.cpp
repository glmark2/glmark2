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
#include "oglsdl.h"
#include "scene.h"
#include "benchmark.h"
#include "options.h"
#include "log.h"

#include <iostream>

#if USE_GL
#include "canvas-x11-glx.h"
#elif USE_GLESv2
#include "canvas-x11-egl.h"
#endif

using std::vector;
using std::map;
using std::string;

static const char *default_benchmarks[] = {
    "build:use-vbo=false",
    "build:use-vbo=true",
    "texture:texture-filter=nearest",
    "texture:texture-filter=linear",
    "texture:texture-filter=mipmap",
    "shading:shading=gouraud",
    "shading:shading=phong",
    NULL
};

void
add_default_benchmarks(vector<Benchmark *> &benchmarks)
{
    for (const char **s = default_benchmarks; *s != NULL; s++)
        benchmarks.push_back(new Benchmark(*s));
}

void
add_custom_benchmarks(vector<Benchmark *> &benchmarks)
{
    for (vector<string>::const_iterator iter = Options::benchmarks.begin();
         iter != Options::benchmarks.end();
         iter++)
    {
        benchmarks.push_back(new Benchmark(*iter));
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
do_benchmark(Canvas &canvas, vector<Benchmark *> &benchmarks)
{
    unsigned score = 0;

    for (vector<Benchmark *>::iterator bench_iter = benchmarks.begin();
         bench_iter != benchmarks.end();
         bench_iter++)
    {
        bool keep_running = true;
        Benchmark *bench = *bench_iter;
        Scene &scene = bench->setup_scene();

        if (!scene.name().empty()) {
            Log::info("%s", scene.info_string().c_str());
            Log::flush();

            while (scene.is_running() &&
                   (keep_running = !canvas.should_quit()))
            {
                canvas.clear();

                scene.draw();
                scene.update();

                canvas.update();
            }

            Log::info(" FPS: %u\n", scene.average_fps());
            score += scene.average_fps();
        }

        bench->teardown_scene();

        if (!keep_running)
            break;
    }

    Log::info("=======================================================\n");
    Log::info("                                  glmark2 Score: %u \n", score);
    Log::info("=======================================================\n");

}

void
do_validation(Canvas &canvas, vector<Benchmark *> &benchmarks)
{
    for (vector<Benchmark *>::iterator bench_iter = benchmarks.begin();
         bench_iter != benchmarks.end();
         bench_iter++)
    {
        Benchmark *bench = *bench_iter;
        Scene &scene = bench->setup_scene();

        if (!scene.name().empty()) {
            Log::info("%s", scene.info_string().c_str());
            Log::flush();

            canvas.clear();
            scene.draw();
            canvas.update();

            string result;
            switch(scene.validate()) {
                case Scene::ValidationSuccess:
                    result = "Success";
                    break;
                case Scene::ValidationFailure:
                    result = "Failure";
                    break;
                case Scene::ValidationUnknown:
                    result = "Unknown";
                    break;
                default:
                    break;
            }

            Log::info(" Validation: %s\n", result.c_str());
        }

        bench->teardown_scene();
    }
}

int main(int argc, char *argv[])
{

    if (!Options::parse_args(argc, argv))
        return 1;

    if (Options::show_help) {
        Options::print_help();
        return 0;
    }

    // Create the canvas
#if USE_GL
    CanvasX11GLX canvas(800, 600);
#elif USE_GLESv2
    CanvasX11EGL canvas(800, 600);
#endif

    // Register the scenes, so they can be looked-up by name
    Benchmark::register_scene(*new SceneDefaultOptions(canvas));
    Benchmark::register_scene(*new SceneBuild(canvas));
    Benchmark::register_scene(*new SceneTexture(canvas));
    Benchmark::register_scene(*new SceneShading(canvas));

    if (Options::list_scenes) {
        list_scenes();
        return 0;
    }

    if (!canvas.init()) {
        Log::error("Error: %s: Could not initialize canvas\n", __FUNCTION__);
        return 1;
    }

    // Add the benchmarks to run
    vector<Benchmark *> benchmarks;

    if (Options::benchmarks.empty())
        add_default_benchmarks(benchmarks);
    else
        add_custom_benchmarks(benchmarks);

    Log::info("=======================================================\n");
    Log::info("    glmark2 %s\n", GLMARK_VERSION);
    Log::info("=======================================================\n");
    canvas.print_info();
    Log::info("=======================================================\n");

    canvas.visible(true);

    if (Options::validate)
        do_validation(canvas, benchmarks);
    else
        do_benchmark(canvas, benchmarks);

    return 0;
}
