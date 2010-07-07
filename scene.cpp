#include "scene.h"

Scene::Scene()
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


