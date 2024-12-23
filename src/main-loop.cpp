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
#include "results-file.h"

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
    scene_setup_status_ = SceneSetupStatusUnknown;
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

            next_benchmark();
        }

        /* If we have found a valid scene, set it up */
        if (bench_iter_ != benchmarks_.end()) {
            before_scene_setup();
            if (!Options::reuse_context)
                canvas_.reset();
            scene_ = &(*bench_iter_)->setup_scene();
            if (!scene_->running()) {
                if (!scene_->supported(false))
                    scene_setup_status_ = SceneSetupStatusUnsupported;
                else
                    scene_setup_status_ = SceneSetupStatusFailure;
            }
            else {
                scene_setup_status_ = SceneSetupStatusSuccess;
            }
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
        (*bench_iter_)->teardown_scene();
        if (scene_setup_status_ == SceneSetupStatusSuccess) {
            score_ += scene_->average_fps();
            benchmarks_run_++;
        }
        log_scene_result();
        scene_ = 0;
        next_benchmark();
    }

    return !should_quit;
}

void
MainLoop::draw()
{
    canvas_.clear();

    scene_->draw();
    scene_->update();

    canvas_.update(scene_->name(), scene_->currentFrame_);
}

void
MainLoop::log_scene_info()
{
    std::string info_string = scene_->info_string();

    Log::info("%s:", info_string.c_str());
    Log::flush();

    ResultsFile &results_file = ResultsFile::get();
    results_file.begin_benchmark();
    results_file.add_field("name", info_string);
}

void
MainLoop::log_scene_result()
{
    static const std::string format_fps(Log::continuation_prefix +
                                        " FPS: %s FrameTime: %s ms");
    static const std::string format_frame(Log::continuation_prefix +
                                          " FrameTime: %s ms");
    static const std::string format_cpu(Log::continuation_prefix +
                                        " (User: %s ms, System: %s ms) CpuBusy: %s%%");
    static const std::string format_shader(Log::continuation_prefix +
                                           " ShaderCompTime: %s ms");
    static const std::string format_unsupported(Log::continuation_prefix +
                                                " Unsupported\n");
    static const std::string format_fail(Log::continuation_prefix +
                                         " Set up failed\n");
    static const std::string format_done(Log::continuation_prefix + " done");
    static const std::string format_newline(Log::continuation_prefix + "\n");
    ResultsFile &results_file = ResultsFile::get();

    if (scene_setup_status_ == SceneSetupStatusSuccess) {
        Scene::Stats stats = scene_->stats();

        if (Options::results & Options::ResultsFps)
        {
            std::string fps =
                Util::toString(static_cast<unsigned>(ceil(1.0 / stats.average_frame_time)));
            std::string frame_time =
                Util::toString(1000.0 * stats.average_frame_time, 3);

            Log::info(format_fps.c_str(), fps.c_str(), frame_time.c_str());
            results_file.add_field("fps", fps);
            results_file.add_field("frame_time", frame_time);
        }

        if (Options::results & Options::ResultsCpu)
        {
            if (!(Options::results & Options::ResultsFps))
            {
                std::string frame_time =
                    Util::toString(1000.0 * stats.average_frame_time, 3);

                Log::info(format_frame.c_str(), frame_time.c_str());
                results_file.add_field("frame_time", frame_time);
            }

            std::string user_time =
                Util::toString(1000.0 * stats.average_user_time, 3);
            std::string system_time =
                Util::toString(1000.0 * stats.average_system_time, 3);
            std::string cpu_busy =
                Util::toString(static_cast<int>(100.0 * stats.cpu_busy_percent));

            Log::info(format_cpu.c_str(),
                      user_time.c_str(), system_time.c_str(), cpu_busy.c_str());
            results_file.add_field("user_time", user_time);
            results_file.add_field("system_time", system_time);
            results_file.add_field("cpu_busy", cpu_busy);
        }

        if (Options::results & Options::ResultsShader)
        {
            std::string shader_time =
                Util::toString(1000.0 * stats.shader_compilation_time, 3);

            Log::info(format_shader.c_str(), shader_time.c_str());
            results_file.add_field("shader_comp_time", shader_time);
        }

        if (Options::results == 0)
        {
            Log::info(format_done.c_str());
        }

        Log::info(format_newline.c_str());
        results_file.add_field("status", "Success");
    }
    else if (scene_setup_status_ == SceneSetupStatusUnsupported) {
        Log::info(format_unsupported.c_str());
        results_file.add_field("status", "Unsupported");
    }
    else {
        Log::info(format_fail.c_str());
        results_file.add_field("status", "Failure");
    }

    results_file.end_benchmark();
}

void
MainLoop::next_benchmark()
{
    bench_iter_++;
    if (bench_iter_ == benchmarks_.end() && Options::run_forever)
        bench_iter_ = benchmarks_.begin();
}

/**********************
 * MainLoopDecoration *
 **********************/

MainLoopDecoration::MainLoopDecoration(Canvas &canvas, const std::vector<Benchmark *> &benchmarks) :
    MainLoop(canvas, benchmarks), show_fps_(false), show_title_(false),
    fps_renderer_(0), title_renderer_(0), last_fps_(0)
{

}

MainLoopDecoration::~MainLoopDecoration()
{
    delete fps_renderer_;
    fps_renderer_ = 0;
    delete title_renderer_;
    title_renderer_ = 0;
}

void
MainLoopDecoration::draw()
{
    static const unsigned int fps_interval = 500000;

    canvas_.clear();

    scene_->draw();
    scene_->update();

    if (show_fps_) {
        uint64_t now = Util::get_timestamp_us();
        if (now - fps_timestamp_ >= fps_interval) {
            last_fps_ = scene_->average_fps();
            fps_renderer_update_text(last_fps_);
            fps_timestamp_ = now;
        }
        fps_renderer_->render();
    }

    if (show_title_)
        title_renderer_->render();

    canvas_.update(scene_->name(), scene_->currentFrame_);
}

void
MainLoopDecoration::before_scene_setup()
{
    delete fps_renderer_;
    fps_renderer_ = 0;
    delete title_renderer_;
    title_renderer_ = 0;
}

void
MainLoopDecoration::after_scene_setup()
{
    const Scene::Option &show_fps_option(scene_->options().find("show-fps")->second);
    const Scene::Option &title_option(scene_->options().find("title")->second);
    show_fps_ = show_fps_option.value == "true";
    show_title_ = !title_option.value.empty();

    if (show_fps_) {
        const Scene::Option &fps_pos_option(scene_->options().find("fps-pos")->second);
        const Scene::Option &fps_size_option(scene_->options().find("fps-size")->second);
        fps_renderer_ = new TextRenderer(canvas_);
        fps_renderer_->position(vec2_from_pos_string(fps_pos_option.value));
        fps_renderer_->size(Util::fromString<float>(fps_size_option.value));
        fps_renderer_update_text(last_fps_);
        fps_timestamp_ = Util::get_timestamp_us();
    }

    if (show_title_) {
        const Scene::Option &title_pos_option(scene_->options().find("title-pos")->second);
        const Scene::Option &title_size_option(scene_->options().find("title-size")->second);
        title_renderer_ = new TextRenderer(canvas_);
        title_renderer_->position(vec2_from_pos_string(title_pos_option.value));
        title_renderer_->size(Util::fromString<float>(title_size_option.value));

        if (title_option.value == "#info#")
            title_renderer_->text(scene_->info_string());
        else if (title_option.value == "#name#")
            title_renderer_->text(scene_->name());
        else if (title_option.value == "#r2d2#")
            title_renderer_->text("Help me, Obi-Wan Kenobi. You're my only hope.");
        else
            title_renderer_->text(title_option.value);
    }
}

void
MainLoopDecoration::fps_renderer_update_text(unsigned int fps)
{
    std::stringstream ss;
    ss << "FPS: " << fps;
    fps_renderer_->text(ss.str());
}

LibMatrix::vec2
MainLoopDecoration::vec2_from_pos_string(const std::string &s)
{
    LibMatrix::vec2 v(0.0, 0.0);
    std::vector<std::string> elems;
    Util::split(s, ',', elems, Util::SplitModeNormal);

    if (elems.size() > 0)
        v.x(Util::fromString<float>(elems[0]));

    if (elems.size() > 1)
        v.y(Util::fromString<float>(elems[1]));

    return v;
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

    canvas_.update(scene_->name(), scene_->currentFrame_);

    scene_->running(false);
}

void
MainLoopValidation::log_scene_result()
{
    static const std::string format(Log::continuation_prefix + " Validation: %s\n");
    ResultsFile &results_file = ResultsFile::get();
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
    results_file.add_field("status", result);

    results_file.end_benchmark();
}
