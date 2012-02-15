/*
 * Copyright © 2012 Linaro Limited
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
 *  Alexandros Frantzis
 */
#include "options.h"
#include "main-loop.h"
#include "util.h"
#include "log.h"

#include <string>
#include <sstream>

/************
 * MainLoop *
 ************/

MainLoop::MainLoop(Canvas &canvas, const std::vector<Benchmark *> &benchmarks) :
    canvas_(canvas), benchmarks_(benchmarks)
{
    reset();
}


void
MainLoop::reset()
{
    scene_ = 0;
    score_ = 0;
    benchmarks_run_ = 0;
    bench_iter_ = benchmarks_.begin();
}

unsigned int
MainLoop::score()
{
    if (benchmarks_run_)
        return score_ / benchmarks_run_;
    else
        return score_;
}

bool
MainLoop::step()
{
    /* Find the next normal scene */
    if (!scene_) {
        /* Find a normal scene */
        while (bench_iter_ != benchmarks_.end()) {
            scene_ = &(*bench_iter_)->scene();

            /* 
             * Scenes with empty names are option-setting scenes.
             * Just set them up and continue with the search.
             */
            if (scene_->name().empty())
                (*bench_iter_)->setup_scene();
            else
                break;

            bench_iter_++;
        }

        /* If we have found a valid scene, set it up */
        if (bench_iter_ != benchmarks_.end()) {
            if (!Options::reuse_context)
                canvas_.reset();
            before_scene_setup();
            scene_ = &(*bench_iter_)->setup_scene();
            after_scene_setup();
            log_scene_info();
        }
        else {
            /* ... otherwise we are done */
            return false;
        }
    }

    bool should_quit = canvas_.should_quit();

    if (scene_ ->running() && !should_quit)
        draw();

    /*
     * Need to recheck whether the scene is still running, because code
     * in draw() may have changed the state.
     */
    if (!scene_->running() || should_quit) {
        score_ += scene_->average_fps();
        log_scene_result();
        (*bench_iter_)->teardown_scene();
        scene_ = 0;
        bench_iter_++;
        benchmarks_run_++;
    }

    return !should_quit;
}

void
MainLoop::draw()
{
    canvas_.clear();

    scene_->draw();
    scene_->update();

    canvas_.update();
}

void
MainLoop::log_scene_info()
{
    Log::info("%s", scene_->info_string().c_str());
    Log::flush();
}

void
MainLoop::log_scene_result()
{
    static const std::string format(Log::continuation_prefix + " FPS: %u\n");
    Log::info(format.c_str(), scene_->average_fps());
}

/**********************
 * MainLoopDecoration *
 **********************/

MainLoopDecoration::MainLoopDecoration(Canvas &canvas, const std::vector<Benchmark *> &benchmarks) :
    MainLoop(canvas, benchmarks), fps_renderer_(0), last_fps_(0)
{

}

MainLoopDecoration::~MainLoopDecoration()
{
    delete fps_renderer_;
    fps_renderer_ = 0;
}

void
MainLoopDecoration::draw()
{
    static const unsigned int fps_interval = 500000;
    uint64_t now = Util::get_timestamp_us();

    canvas_.clear();

    scene_->draw();
    scene_->update();

    if (now - fps_timestamp_ >= fps_interval) {
        last_fps_ = scene_->average_fps();
        fps_renderer_update_text(last_fps_);
        fps_timestamp_ = now;
    }
    fps_renderer_->render();

    canvas_.update();
}

void
MainLoopDecoration::before_scene_setup()
{
    delete fps_renderer_;
    fps_renderer_ = new TextRenderer(canvas_);
    fps_renderer_update_text(last_fps_);
    fps_timestamp_ = Util::get_timestamp_us();
}

void
MainLoopDecoration::fps_renderer_update_text(unsigned int fps)
{
    std::stringstream ss;
    ss << "FPS: " << fps;
    fps_renderer_->text(ss.str());
}

/**********************
 * MainLoopValidation *
 **********************/

MainLoopValidation::MainLoopValidation(Canvas &canvas, const std::vector<Benchmark *> &benchmarks) :
        MainLoop(canvas, benchmarks)
{
}

void
MainLoopValidation::draw()
{
    /* Draw only the first frame of the scene and stop */
    canvas_.clear();

    scene_->draw();

    canvas_.update();

    scene_->running(false);
}

void
MainLoopValidation::log_scene_result()
{
    static const std::string format(Log::continuation_prefix + " Validation: %s\n");
    std::string result;

    switch(scene_->validate()) {
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

    Log::info(format.c_str(), result.c_str());
}
