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
#include <string>
#include <fstream>
#include "canvas-android.h"
#include "benchmark.h"
#include "options.h"
#include "log.h"
#include "util.h"
#include "default-benchmarks.h"
#include "main-loop.h"

static Canvas *g_canvas;
static MainLoop *g_loop;

class MainLoopAndroid : public MainLoop
{
public:
    MainLoopAndroid(Canvas &canvas) :
        MainLoop(canvas) {}

    virtual void log_scene_info() {}

    virtual void log_scene_result()
    {
        Log::info("%s FPS: %u", scene_->info_string().c_str(),
                                scene_->average_fps());
    }
};

class MainLoopDecorationAndroid : public MainLoopDecoration
{
public:
    MainLoopDecorationAndroid(Canvas &canvas) :
        MainLoopDecoration(canvas) {}

    virtual void log_scene_info() {}

    virtual void log_scene_result()
    {
        Log::info("%s FPS: %u", scene_->info_string().c_str(),
                                scene_->average_fps());
    }
};

/** 
 * Converts an std::vector containing arguments to argc,argv.
 */
static void
arg_vector_to_argv(const std::vector<std::string> &arguments, int &argc, char **&argv)
{
    argc = arguments.size() + 1;
    argv = new char* [argc];
    argv[0] = strdup("glmark2");

    for (unsigned int i = 0; i < arguments.size(); i++)
        argv[i + 1] = strdup(arguments[i].c_str());
}

/** 
 * Populates the command line arguments from the arguments file.
 * 
 * @param argc the number of arguments
 * @param argv the argument array
 */
static void
get_args_from_file(const std::string &arguments_file, int &argc, char **&argv)
{
    std::vector<std::string> arguments;
    std::ifstream ifs(arguments_file.c_str());

    if (!ifs.fail()) {
        std::string line;
        while (getline(ifs, line)) {
            if (!line.empty())
                Util::split(line, ' ', arguments);
        }
    }

    arg_vector_to_argv(arguments, argc, argv);
}

/** 
 * Populates the command line arguments from the arguments file.
 * 
 * @param argc the number of arguments
 * @param argv the argument array
 */
static void
get_args_from_string(const std::string &args_str, int &argc, char **&argv)
{
    std::vector<std::string> arguments;
    Util::split(args_str, ' ', arguments);

    arg_vector_to_argv(arguments, argc, argv);
}

/** 
 * Releases the command line arguments.
 * 
 * @param argc the number of arguments
 * @param argv the argument array
 */
static void
release_args(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
        free(argv[i]);

    delete[] argv;
}

void
Java_org_linaro_glmark2_Glmark2Renderer_nativeInit(JNIEnv* env, jclass clazz,
                                                   jobject asset_manager,
                                                   jstring args)
{
    static_cast<void>(clazz);
    static const std::string arguments_file("/data/glmark2/args");
    int argc = 0;
    char **argv = 0;

    /* Load arguments from argument string or arguments file and parse them */
    if (args) {
        if (env->GetStringUTFLength(args) > 0) {
            const char *args_c_str = env->GetStringUTFChars(args, 0);
            if (args_c_str) {
                get_args_from_string(std::string(args_c_str), argc, argv);
                env->ReleaseStringUTFChars(args, args_c_str);
            }
        }
    }
    else {
        get_args_from_file(arguments_file, argc, argv);
    }

    Options::parse_args(argc, argv);
    release_args(argc, argv);

    /* Force reuse of EGL/GL context */
    Options::reuse_context = true;

    Log::init("glmark2", Options::show_debug);
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

    if (Options::show_fps)
        g_loop = new MainLoopDecorationAndroid(*g_canvas);
    else
        g_loop = new MainLoopAndroid(*g_canvas);

    g_loop->add_benchmarks();
}

void
Java_org_linaro_glmark2_Glmark2Renderer_nativeResize(JNIEnv* env,
                                                     jclass clazz,
                                                     jint w,
                                                     jint h)
{
    static_cast<void>(env);
    static_cast<void>(clazz);

    Log::debug("Resizing to %d x %d\n", w, h);
    g_canvas->resize(w, h);
}

void
Java_org_linaro_glmark2_Glmark2Renderer_nativeDone(JNIEnv* env)
{
    static_cast<void>(env);

    delete g_loop;
    delete g_canvas;
}

jboolean
Java_org_linaro_glmark2_Glmark2Renderer_nativeRender(JNIEnv* env)
{
    static_cast<void>(env);

    if (!g_loop->step()) {
        Log::info("glmark2 Score: %u\n", g_loop->score());
        return false;
    }

    return true;
}

static JNINativeMethod glmark2_native_methods[] = {
    {
        "nativeInit",
        "(Landroid/content/res/AssetManager;Ljava/lang/String;)V",
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
    static_cast<void>(reserved);
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
