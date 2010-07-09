#include "scene.h"

SceneBuild::~SceneBuild()
{
}

int SceneBuild::load()
{
    Model model;
    
    if(!model.load_3ds(GLMARK_DATA_PATH"data/models/horse.3ds"))
        return 0;
        
    model.calculate_normals();
    model.convert_to_mesh(&mMesh);
    
    mMesh.build_vbo();
    
    mShader.load(GLMARK_DATA_PATH"data/shaders/light-basic.vert",
                 GLMARK_DATA_PATH"data/shaders/light-basic.frag");

    mRotationSpeed = 36.0f;
    mRotation = 0.0;
    
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

    memset(mAverageFPS, 0, mPartsQty * sizeof(*mAverageFPS));
    
    mCurrentPart = 0;
    
    return 1;
}

void SceneBuild::start()
{
    GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {20.0f, 20.0f, 10.0f, 1.0f};

    mShader.use();

    // Load lighting and material uniforms
    glUniform4fv(mShader.mLocations.LightSourcePosition, 1, lightPosition);

    glUniform3fv(mShader.mLocations.LightSourceAmbient, 1, lightAmbient);
    glUniform3fv(mShader.mLocations.LightSourceDiffuse, 1, lightDiffuse);

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
    // Load the ModelViewProjectionMatrix uniform in the shader
    Matrix4f model_view(1.0f, 1.0f, 1.0f);
    Matrix4f model_view_proj(mScreen.mProjection);

    model_view.translate(0.0f, 0.0f, -2.5f);
    model_view.rotate(2 * M_PI * mRotation / 360.0, 0.0f, 1.0f, 0.0f);
    model_view_proj *= model_view;

    glUniformMatrix4fv(mShader.mLocations.ModelViewProjectionMatrix, 1,
                       GL_FALSE, model_view_proj.m);

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    model_view.invert().transpose();
    glUniformMatrix4fv(mShader.mLocations.NormalMatrix, 1,
                       GL_FALSE, model_view.m);

    switch(mCurrentPart)
    {
    case 0:
        mMesh.render_array_attrib();
        break;
    case 1:
        mMesh.render_vbo_attrib();
        break;
    }    
}
