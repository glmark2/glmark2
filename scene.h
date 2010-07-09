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
    
    unsigned mPartsQty;         // How many parts for the scene
    unsigned mCurrentPart;      // The current part being rendered
    double *mPartDuration;      // Duration per part in seconds

    double mLastTime, mCurrentTime, mDt;
    unsigned mCurrentFrame;
    bool mRunning;
    
    unsigned *mAverageFPS;      // Average FPS per part
    float *mScoreScale;
    unsigned mScore;            // Score
    
    void calculate_score();
    
protected:
    double mStartTime;
    double mElapsedTime;

    Screen &mScreen;
};

class SceneBuild : public Scene
{
public:
    SceneBuild(Screen &pScreen) : Scene(pScreen) {}
    int load();
    void start();
    void update();
    void draw();
    
    ~SceneBuild();

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
    void start();
    void update();
    void draw();
    
    ~SceneTexture();
    
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
    void start();
    void update();
    void draw();
    
    ~SceneShading();
    
    Shader mShader[2];

    Mesh mMesh;
    float mRotation;
    float mRotationSpeed;
};

#endif
