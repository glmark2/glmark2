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

using std::string;
using std::map;

class Scene
{
public:
    ~Scene();

    struct Option {
        Option(string nam, string val, string desc) :
            name(nam), value(val), default_value(val), description(desc) {}
        Option() {}
        string name;
        string value;
        string default_value;
        string description;
    };

    virtual int load();
    virtual void unload();
    virtual void start();
    virtual void update();
    virtual void draw();

    unsigned calculate_score();
    bool is_running();

    const string &name() { return mName; }
    bool set_option(const string &opt, const string &val);
    void reset_options();
    const map<string, Option> &options() { return mOptions; }

protected:
    Scene(Screen &pScreen, const string &name);

    unsigned mPartsQty;         // How many parts for the scene
    unsigned mCurrentPart;      // The current part being rendered
    double *mPartDuration;      // Duration per part in seconds

    double mLastTime, mCurrentTime, mDt;
    unsigned mCurrentFrame;
    bool mRunning;

    unsigned *mAverageFPS;      // Average FPS per part
    float *mScoreScale;

    double mStartTime;
    double mElapsedTime;

    Screen &mScreen;
    string mName;
    map<string, Option> mOptions;
};

class SceneBuild : public Scene
{
public:
    SceneBuild(Screen &pScreen);
    int load();
    void unload();
    void start();
    void update();
    void draw();

    ~SceneBuild();

protected:
    Shader mShader;

    Mesh mMesh;
    float mRotation;
    float mRotationSpeed;
};

class SceneTexture : public Scene
{
public:
    SceneTexture(Screen &pScreen);
    int load();
    void unload();
    void start();
    void update();
    void draw();

    ~SceneTexture();

protected:
    Shader mShader;

    Mesh mCubeMesh;
    GLuint mTexture[3];
    Vector3f mRotation;
    Vector3f mRotationSpeed;
};

class SceneShading : public Scene
{
public:
    SceneShading(Screen &pScreen);
    int load();
    void unload();
    void start();
    void update();
    void draw();

    ~SceneShading();

protected:
    Shader mShader[2];

    Mesh mMesh;
    float mRotation;
    float mRotationSpeed;
};

#endif
