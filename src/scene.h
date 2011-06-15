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
#ifndef GLMARK2_SCENE_H_
#define GLMARK2_SCENE_H_

#include "oglsdl.h"

#include "mesh.h"
#include "model.h"
#include "texture.h"
#include "shader.h"

#include <math.h>

#include <string>
#include <map>

class Scene
{
public:
    ~Scene();

    struct Option {
        Option(const std::string &nam, const std::string &val, const std::string &desc) :
            name(nam), value(val), default_value(val), description(desc) {}
        Option() {}
        std::string name;
        std::string value;
        std::string default_value;
        std::string description;
    };

    enum ValidationResult {
        ValidationFailure,
        ValidationSuccess,
        ValidationUnknown
    };

    // load() and unload() handle option-independent configuration.
    // It should be safe to call these only once per program execution,
    // although you may choose to do so more times to better manage
    // resource consumption.
    virtual int load();
    virtual void unload();

    // setup() and teardown() handle option-dependent configuration and
    // also prepare a scene for a benchmark run.
    // They should be called just before and after running a scene/benchmark.
    virtual void setup();
    virtual void teardown();

    virtual void update();
    virtual void draw();
    virtual std::string info_string(const std::string &title = "");

    unsigned average_fps();
    bool is_running();

    const std::string &name() { return mName; }
    bool set_option(const std::string &opt, const std::string &val);
    void reset_options();
    const std::map<std::string, Option> &options() { return mOptions; }

    static Scene &dummy()
    {
        static Scene dummy_scene(Screen::dummy(), "");
        return dummy_scene;
    }

    virtual ValidationResult validate() { return ValidationUnknown; }

protected:
    Scene(Screen &pScreen, const std::string &name);
    std::string construct_title(const std::string &title);
    double pixel_value_distance(Screen::Pixel p1, Screen::Pixel p2,
                                bool use_alpha=false);

    Screen &mScreen;
    std::string mName;
    std::map<std::string, Option> mOptions;

    double mStartTime;
    double mLastUpdateTime;
    unsigned mCurrentFrame;
    unsigned mAverageFPS;      // Average FPS of run

    bool mRunning;
    double mDuration;      // Duration of run in seconds
};

class SceneBuild : public Scene
{
public:
    SceneBuild(Screen &pScreen);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~SceneBuild();

protected:
    Shader mShader;

    Mesh mMesh;
    float mRotation;
    float mRotationSpeed;
    bool mUseVbo;
};

class SceneTexture : public Scene
{
public:
    SceneTexture(Screen &pScreen);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();

    ~SceneTexture();

protected:
    Shader mShader;

    Mesh mCubeMesh;
    GLuint mTexture;
    Vector3f mRotation;
    Vector3f mRotationSpeed;
};

class SceneShading : public Scene
{
public:
    SceneShading(Screen &pScreen);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();

    ~SceneShading();

protected:
    Shader mShader;

    Mesh mMesh;
    float mRotation;
    float mRotationSpeed;
};

#endif
