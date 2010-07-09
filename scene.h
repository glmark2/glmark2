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
