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
