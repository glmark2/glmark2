/*
 * Copyright © 2011 Linaro Limited
 * Copyright © 2011 0xlab - http://0xlab.org/
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
 *  Jim Huang (Strict JNI registration using JNI_OnLoad())
 */
#include <assert.h>
#include <jni.h>
#include <vector>
#include "canvas-android.h"
#include "benchmark.h"
#include "options.h"
#include "log.h"
#include "util.h"
#include "default-benchmarks.h"

static Canvas *g_canvas;
static std::vector<Benchmark *> g_benchmarks;

static void
add_default_benchmarks(std::vector<Benchmark *> &benchmarks)
{
    const std::vector<std::string> &default_benchmarks = DefaultBenchmarks::get();

    for (std::vector<std::string>::const_iterator iter = default_benchmarks.begin();
         iter != default_benchmarks.end();
         iter++)
    {
        benchmarks.push_back(new Benchmark(*iter));
    }
}

void
Java_org_linaro_glmark2_Glmark2Renderer_nativeInit(JNIEnv* env, jclass clazz,
                                                   jobject asset_manager)
{
    Util::android_set_asset_manager(AAssetManager_fromJava(env, asset_manager));

    g_canvas = new CanvasAndroid(100, 100);
    g_canvas->init();

    Log::info("glmark2 %s\n", GLMARK_VERSION);
    g_canvas->print_info();

    Benchmark::register_scene(*new SceneDefaultOptions(*g_canvas));
    Benchmark::register_scene(*new SceneBuild(*g_canvas));
    Benchmark::register_scene(*new SceneTexture(*g_canvas));
    Benchmark::register_scene(*new SceneShading(*g_canvas));
    Benchmark::register_scene(*new SceneConditionals(*g_canvas));
    Benchmark::register_scene(*new SceneFunction(*g_canvas));
    Benchmark::register_scene(*new SceneLoop(*g_canvas));
    Benchmark::register_scene(*new SceneBump(*g_canvas));
    Benchmark::register_scene(*new SceneEffect2D(*g_canvas));
    Benchmark::register_scene(*new ScenePulsar(*g_canvas));
    Benchmark::register_scene(*new SceneDesktop(*g_canvas));
    Benchmark::register_scene(*new SceneBuffer(*g_canvas));

    add_default_benchmarks(g_benchmarks);
}

void
Java_org_linaro_glmark2_Glmark2Renderer_nativeResize(JNIEnv* env,
                                                     jclass clazz,
                                                     jint w,
                                                     jint h)
{
    Log::debug("Resizing to %d x %d\n", w, h);
    g_canvas->resize(w, h);
}

void
Java_org_linaro_glmark2_Glmark2Renderer_nativeDone(JNIEnv* env)
{
    delete g_canvas;
}

jboolean
Java_org_linaro_glmark2_Glmark2Renderer_nativeRender(JNIEnv* env)
{
    static std::vector<Benchmark *>::iterator bench_iter = g_benchmarks.begin();
    static Scene *scene = 0;
    static unsigned int score = 0;
    static unsigned int benchmarks_run = 0;

    if (!scene) {
        /* Find the next normal scene */
        while (bench_iter != g_benchmarks.end()) {
            scene = &(*bench_iter)->setup_scene();
            if (!scene->name().empty())
                break;
            bench_iter++;
        }

        if (bench_iter == g_benchmarks.end()) {
            if (benchmarks_run)
                score /= benchmarks_run;
            Log::info("glmark2 Score: %u\n", score);
            /* Reset the rendering state, in case we get called again */
            bench_iter = g_benchmarks.begin();
            score = 0;
            benchmarks_run = 0;
            return false;
        }
    }

    if (scene->is_running()) {
        g_canvas->clear();

        scene->draw();
        scene->update();
    }

    /* 
     * Need to recheck whether screen is running, because scene->update()
     * may have changed the state.
     */
    if (!scene->is_running()) {
        Log::info("%s FPS: %u", scene->info_string().c_str(), scene->average_fps());
        score += scene->average_fps();
        (*bench_iter)->teardown_scene();
        scene = 0;
        bench_iter++;
        benchmarks_run++;
    }

    return true;
}

static JNINativeMethod glmark2_native_methods[] = {
    {
        "nativeInit",
        "(Landroid/content/res/AssetManager;)V",
        reinterpret_cast<void*>(Java_org_linaro_glmark2_Glmark2Renderer_nativeInit)
    },
    {
        "nativeResize",
        "(II)V",
        reinterpret_cast<void*>(Java_org_linaro_glmark2_Glmark2Renderer_nativeResize)
    },
    {
        "nativeDone",
        "()V",
        reinterpret_cast<void*>(Java_org_linaro_glmark2_Glmark2Renderer_nativeDone)
    },
    {
        "nativeRender",
        "()Z",
        reinterpret_cast<void*>(Java_org_linaro_glmark2_Glmark2Renderer_nativeRender)
    }
};

static int
register_native_methods(JNIEnv* env, const char* className,
                        JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        Log::error("Native registration unable to find class '%s'\n",
                   className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        Log::error("RegisterNatives failed for '%s'\n", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static int
register_natives(JNIEnv *env)
{
    const char* const class_path_name = "org/linaro/glmark2/Glmark2Renderer";
    return register_native_methods(env, class_path_name,
                                   glmark2_native_methods,
                                   sizeof(glmark2_native_methods) /
                                   sizeof(glmark2_native_methods[0]));
}

/*
 * Returns the JNI version on success, -1 on failure.
 */
extern "C" jint
JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_4) != JNI_OK) {
        Log::error("JNI_OnLoad: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (!register_natives(env)) {
        Log::error("JNI_OnLoad: glmark2 native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}
