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

static const char *default_benchmarks[] = {
    "build:use-vbo=false",
    "build:use-vbo=true",
    "texture:texture-filter=nearest",
    "texture:texture-filter=linear",
    "texture:texture-filter=mipmap",
    "shading:shading=gouraud",
    "shading:shading=blinn-phong-inf",
    "shading:shading=phong",
    "bump:bump-render=high-poly",
    "bump:bump-render=normals",
    "effect2d:kernel=0,1,0;1,-4,1;0,1,0;",
    "effect2d:kernel=1,1,1,1,1;1,1,1,1,1;1,1,1,1,1;",
    "pulsar:quads=5:texture=false:light=false",
    "desktop:windows=4:effect=blur:blur-radius=5:passes=1:separable=true",
    "conditionals:vertex-steps=0:fragment-steps=0",
    "conditionals:vertex-steps=0:fragment-steps=5",
    "conditionals:vertex-steps=5:fragment-steps=0",
    "function:fragment-steps=5:fragment-complexity=low",
    "function:fragment-steps=5:fragment-complexity=medium",
    "loop:vertex-steps=5:fragment-steps=5:fragment-loop=false",
    "loop:vertex-steps=5:fragment-steps=5:fragment-uniform=false",
    "loop:vertex-steps=5:fragment-steps=5:fragment-uniform=true",
    NULL
};

static Canvas *g_canvas;
static std::vector<Benchmark *> g_benchmarks;

static void
add_default_benchmarks(std::vector<Benchmark *> &benchmarks)
{
    for (const char **s = default_benchmarks; *s != NULL; s++)
        benchmarks.push_back(new Benchmark(*s));
}

void
Java_org_linaro_glmark2_Glmark2Renderer_nativeInit(JNIEnv* env, jclass clazz,
                                                   jobject asset_manager)
{
    g_canvas = new CanvasAndroid(100, 100);
    g_canvas->init();

    Log::info("glmark2\n");
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
    Benchmark::register_scene(*new SceneDesktop(canvas));

    add_default_benchmarks(g_benchmarks);

    Util::android_set_asset_manager(AAssetManager_fromJava(env, asset_manager));
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

    if (!scene) {
        if (bench_iter != g_benchmarks.end()) {
            scene = &(*bench_iter)->setup_scene();
        }
        else
            return false;
    }

    g_canvas->clear();

    scene->draw();
    scene->update();

    if (!scene->is_running()) {
        (*bench_iter)->teardown_scene();
        Log::info("%s FPS: %u", scene->info_string().c_str(), scene->average_fps());
        scene = 0;
        bench_iter++;
    }

    return true;
}

static JNINativeMethod glmark2_native_methods[] = {
    {"nativeInit",   "(Landroid/content/res/AssetManager;)V",
                     (void*)Java_org_linaro_glmark2_Glmark2Renderer_nativeInit},
    {"nativeResize", "(II)V",
                     (void*)Java_org_linaro_glmark2_Glmark2Renderer_nativeResize},
    {"nativeDone",   "()V",
                     (void*)Java_org_linaro_glmark2_Glmark2Renderer_nativeDone},
    {"nativeRender", "()Z",
                     (void*)Java_org_linaro_glmark2_Glmark2Renderer_nativeRender}
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

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
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
