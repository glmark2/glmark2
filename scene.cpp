#include "scene.h"

Scene::Scene(Screen &pScreen) :
    mScreen(pScreen)
{
}

Scene::~Scene()
{
    delete [] mPartDuration;
    delete [] mAverageFPS;
    delete [] mScoreScale;
}

void Scene::calculate_score()
{
    mScore = 0;
    for(unsigned i = 0; i < mPartsQty; i++)
        mScore += mAverageFPS[i] * mScoreScale[i];
}


