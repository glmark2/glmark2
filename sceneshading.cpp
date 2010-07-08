#include "scene.h"

SceneShading::~SceneShading()
{
}

int SceneShading::load()
{
    Model model;
    
    if(!model.load_3ds("data/models/cat.3ds"))
        return 0;
        
    model.calculate_normals();
    model.convert_to_mesh(&mMesh);
    
    mMesh.build_vbo();
    
    mShader[0].load("data/shaders/light-basic.vert", "data/shaders/light-basic.frag");
    mShader[1].load("data/shaders/light-advanced.vert", "data/shaders/light-advanced.frag");

    mRotationSpeed = 36.0f;
    mRotation = 0.0f;
    
    mRunning = false;
    
    mPartsQty = 2;
    mPartDuration = new double[mPartsQty];
    mAverageFPS = new unsigned[mPartsQty];
    mScoreScale = new float[mPartsQty];

    mScoreScale[0] = 0.534f;
    mScoreScale[1] = 0.532f;
    
    mScore = 0;
    
    mPartDuration[0] = 10.0;
    mPartDuration[1] = 10.0;
    
    mCurrentPart = 0;
    
    return 1;
}

void SceneShading::start()
{
    GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {20.0f, 20.0f, 10.0f, 1.0f};

    float no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float mat_diffuse[] = {0.1f, 0.5f, 0.8f, 1.0f};
    float mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float high_shininess = 100.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, high_shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    switch(mCurrentPart)
    {
    case 0:
        glDisable(GL_TEXTURE_2D);
        mShader[0].use();
        break;
    case 1:
        glDisable(GL_TEXTURE_2D);
        mShader[1].use();
        break;
    };
    
    mCurrentFrame = 0;
    mRunning = true;
    mStartTime = SDL_GetTicks() / 1000.0;
    mLastTime = mStartTime;
}

void SceneShading::update()
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
            printf("Shading\n");
            printf("    GLSL per vertex lighting FPS: %u\n", mAverageFPS[mCurrentPart]);
            break;
        case 1:
            printf("    GLSL per pixel lighting  FPS: %u\n", mAverageFPS[mCurrentPart]);
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

void SceneShading::draw()
{
    glTranslatef(0.0f, 0.0f,-5.0f);
    glRotated(mRotation, 0.0f, 1.0, 0.0f);

    glColor3f(0.0f, 1.0f, 1.0f);

    mMesh.render_vbo();
}
