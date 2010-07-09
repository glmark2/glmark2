#include "scene.h"
#include "matrix.h"

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

    mShader.load(GLMARK_DATA_PATH"data/shaders/light-basic.vert",
                 GLMARK_DATA_PATH"data/shaders/light-basic-tex.frag");
    
    mRotationSpeed = Vector3f(36.0f, 36.0f, 36.0f);
    
    mRunning = false;
    
    mPartsQty = 3;
    mPartDuration = new double[mPartsQty];
    mAverageFPS = new unsigned[mPartsQty];
    mScoreScale = new float[mPartsQty];
    
    mScoreScale[0] = 0.471f;
    mScoreScale[1] = 0.533f;
    mScoreScale[2] = 0.405f;
    
    mPartDuration[0] = 10.0;
    mPartDuration[1] = 10.0;
    mPartDuration[2] = 10.0;
    
    memset(mAverageFPS, 0, mPartsQty * sizeof(*mAverageFPS));

    mCurrentPart = 0;
    
    return 1;
}

void SceneTexture::unload()
{
    mShader.remove();
    mShader.unload();
}

void SceneTexture::start()
{
    GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {20.0f, 20.0f, 10.0f, 1.0f};
    GLfloat materialColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    mShader.use();

    // Load lighting and material uniforms
    glUniform4fv(mShader.mLocations.LightSourcePosition, 1, lightPosition);

    glUniform3fv(mShader.mLocations.LightSourceAmbient, 1, lightAmbient);
    glUniform3fv(mShader.mLocations.LightSourceDiffuse, 1, lightDiffuse);

    glUniform3fv(mShader.mLocations.MaterialColor, 1, materialColor);

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
        mCurrentPart++;
        if(mCurrentPart >= mPartsQty)
            mRunning = false;
        else
            start();
    }
    
    mRotation += mRotationSpeed * mDt;
    
    mCurrentFrame++;
}

void SceneTexture::draw()
{
    // Load the ModelViewProjectionMatrix uniform in the shader
    Matrix4f model_view(1.0f, 1.0f, 1.0f);
    Matrix4f model_view_proj(mScreen.mProjection);

    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view.rotate(2 * M_PI * mRotation.x / 360.0, 1.0f, 0.0f, 0.0f);
    model_view.rotate(2 * M_PI * mRotation.y / 360.0, 0.0f, 1.0f, 0.0f);
    model_view.rotate(2 * M_PI * mRotation.z / 360.0, 0.0f, 0.0f, 1.0f);
    model_view_proj *= model_view;

    glUniformMatrix4fv(mShader.mLocations.ModelViewProjectionMatrix, 1,
                       GL_FALSE, model_view_proj.m);

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    model_view.invert().transpose();
    glUniformMatrix4fv(mShader.mLocations.NormalMatrix, 1,
                       GL_FALSE, model_view.m);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture[mCurrentPart]);

    mCubeMesh.render_vbo_attrib();
}
