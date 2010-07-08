#include "scene.h"

SceneTexture::~SceneTexture()
{
    for(unsigned i = 0; i < 3; i++)
        glDeleteTextures(1, &mTexture[i]);
}

int SceneTexture::load()
{
    Model model;
    
    if(!model.load_3ds(GLMARK_DATA_PATH"data/models/cube.3ds"))
        return 0;
    
    if(!load_texture(GLMARK_DATA_PATH"data/textures/crate-base.bmp", mTexture))
        return 0;
    
    model.calculate_normals();
    model.convert_to_mesh(&mCubeMesh);
    mCubeMesh.build_vbo();
    
    mRotationSpeed = Vector3f(36.0f, 36.0f, 36.0f);
    
    mRunning = false;
    
    mPartsQty = 3;
    mPartDuration = new double[mPartsQty];
    mAverageFPS = new unsigned[mPartsQty];
    mScoreScale = new float[mPartsQty];
    
    mScoreScale[0] = 0.471f;
    mScoreScale[1] = 0.533f;
    mScoreScale[2] = 0.405f;
    
    mScore = 0;
    
    mPartDuration[0] = 10.0;
    mPartDuration[1] = 10.0;
    mPartDuration[2] = 10.0;
    
    mCurrentPart = 0;
    
    return 1;
}

void SceneTexture::start()
{
    GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {20.0f, 20.0f, 10.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    
    glEnable(GL_TEXTURE_2D);
    
    mCurrentFrame = 0;
    mRunning = true;
    mStartTime = SDL_GetTicks() / 1000.0;
    mLastTime = mStartTime;
}

void SceneTexture::update()
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
            printf("Texture filtering\n");
            printf("    Nearest                       FPS: %u\n",  mAverageFPS[mCurrentPart]);
            break;
        case 1:
            printf("    Linear                        FPS: %u\n",  mAverageFPS[mCurrentPart]);
            break;
        case 2:
            printf("    Mipmapped                     FPS: %u\n",  mAverageFPS[mCurrentPart]);
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

void SceneTexture::draw()
{
    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(0.0f, 0.0f, -4.0f);
    
    glRotatef(mRotation.x, 1.0f, 0.0f, 0.0f);
    glRotatef(mRotation.y, 0.0f, 1.0f, 0.0f);
    glRotatef(mRotation.z, 0.0f, 0.0f, 1.0f);
    
    switch(mCurrentPart)
    {
    case 0:
        glBindTexture(GL_TEXTURE_2D, mTexture[0]);
        mCubeMesh.render_vbo();
        break;
    case 1:
        glBindTexture(GL_TEXTURE_2D, mTexture[1]);
        mCubeMesh.render_vbo();
    case 2:
        glBindTexture(GL_TEXTURE_2D, mTexture[2]);
        mCubeMesh.render_vbo();
        break;
    }    
}
