#include "scene.h"

int SceneBuild::load()
{
    Model model;
    
    if(!model.load_3ds(GLMARK_DATA_PATH"data/models/horse.3ds"))
        return 0;
        
    model.calculate_normals();
    model.convert_to_mesh(&mMesh);
    
    mMesh.build_vbo();

    mRotationSpeed = 36.0f;
    
    mRunning = false;
    
    mPartsQty = 2;
    mPartDuration = new double[mPartsQty];
    mAverageFPS = new unsigned[mPartsQty];
    mScoreScale = new float[mPartsQty];

    mScoreScale[0] = 1.898f;
    mScoreScale[1] = 0.540f;

    mScore = 0;
    
    mPartDuration[0] = 10.0;
    mPartDuration[1] = 10.0;
    
    mCurrentPart = 0;
    
    return 1;
}

void SceneBuild::start()
{
    GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {20.0f, 20.0f, 10.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    mCurrentFrame = 0;
    mRunning = true;
    mStartTime = SDL_GetTicks() / 1000.0;
    mLastTime = mStartTime;
}

void SceneBuild::update()
{
    mCurrentTime = SDL_GetTicks() / 1000.0;
    mDt = mCurrentTime - mLastTime;
    mLastTime = mCurrentTime;
    
    mElapsedTime = mCurrentTime - mStartTime;
    
    if(mElapsedTime >= mPartDuration[mCurrentPart])
    {
        mAverageFPS[mCurrentPart] = mCurrentFrame / mElapsedTime;
        
        switch(mCurrentPart)
        {
        case 0:
            printf("Precompilation\n");
            printf("    Vertex array                  FPS: %u\n", mAverageFPS[mCurrentPart]);
            break;
        case 1:
            printf("    Vertex buffer object          FPS: %u\n", mAverageFPS[mCurrentPart]);
            break;
        }
        mScore += mAverageFPS[mCurrentPart];
        mCurrentPart++;
        start();
        if(mCurrentPart >= mPartsQty)
            mRunning = false;
    }
    
    mRotation += mRotationSpeed * mDt;
    
    mCurrentFrame++;
}

void SceneBuild::draw()
{
    glTranslatef(0.0f, 0.0f,-2.5f);
    glRotated(mRotation, 0.0f, 1.0, 0.0f);

    glColor3f(0.0f, 1.0f, 1.0f);
    switch(mCurrentPart)
    {
    case 0:
        mMesh.render_array();
        break;
    case 1:
        mMesh.render_vbo();
        break;
    }    
}
