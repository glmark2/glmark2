#include "scene.h"

Scene::Scene(Screen &pScreen) :
    mScreen(pScreen)
{
    mPartsQty = 0;
    mCurrentPart = 0;
    mPartDuration = 0;

    mLastTime = 0;
    mCurrentTime = 0;
    mDt = 0;
    mCurrentFrame = 0;
    mRunning = false;
    
    mAverageFPS = 0;
    mScoreScale = 0;

    mStartTime = 0;
    mElapsedTime = 0;
}

Scene::~Scene()
{
    delete [] mPartDuration;
    delete [] mAverageFPS;
    delete [] mScoreScale;
}

int Scene::load()
{
    return 1;
}

void Scene::unload()
{
}

void Scene::start()
{
}

void Scene::update()
{
}

void Scene::draw()
{
}

unsigned Scene::calculate_score()
{
    unsigned mScore = 0;

    for(unsigned i = 0; i < mPartsQty; i++)
        mScore += mAverageFPS[i] * mScoreScale[i];

    return mScore;
}


bool Scene::is_running()
{
    return mRunning;
}
