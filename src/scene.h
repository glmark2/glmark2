/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010 Linaro
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 3, as published by the Free
 * Software Foundation.
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
#ifndef _SCENE_H
#define _SCENE_H

#include "oglsdl.h"

#include "mesh.h"
#include "model.h"
#include "texture.h"
#include "shader.h"

#include <math.h>

class Scene
{
public:
    Scene(Screen &pScreen);
    ~Scene();

    virtual int load();
    virtual void unload();
    virtual void start();
    virtual void update();
    virtual void draw();

    unsigned calculate_score();
    bool is_running();

protected:
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
};

class SceneBuild : public Scene
{
public:
    SceneBuild(Screen &pScreen) : Scene(pScreen) {}
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
    SceneTexture(Screen &pScreen) : Scene(pScreen) {}
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
    SceneShading(Screen &pScreen) : Scene(pScreen) {}
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
