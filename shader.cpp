#include "shader.h"

char *readShaderFile(const char *FileName)
{
    FILE *fp;
    char *DATA = NULL;

    int flength = 0;

    fp = fopen(FileName,"rt");

    fseek(fp, 0, SEEK_END);

    flength = ftell(fp);

    rewind(fp);


    DATA = (char *)malloc(sizeof(char) * (flength+1));
    flength = fread(DATA, sizeof(char), flength, fp);
    DATA[flength] = '\0';

    fclose(fp);

    return DATA;
}

Shader::~Shader()
{
    remove();
}

void Shader::load(const char *pVertexShaderFileName, const char *pFragmentShaderFileName)
{
    char *vertex_shader_source, *fragment_shader_source;
    char msg[512];

    mVertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER);
    mFragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);

    vertex_shader_source = readShaderFile(pVertexShaderFileName);

    fragment_shader_source = readShaderFile(pFragmentShaderFileName);

    const char *vs = vertex_shader_source;
    const char *fs = fragment_shader_source;

    glShaderSourceARB(mVertexShader, 1, &vs, NULL);
    glShaderSourceARB(mFragmentShader, 1, &fs, NULL);

    free(vertex_shader_source);
    free(fragment_shader_source);

    glCompileShaderARB(mVertexShader);
    glGetShaderInfoLog(mVertexShader, sizeof msg, NULL, msg);
    if (strlen(msg) > 0)
        printf("%s: %s", pVertexShaderFileName, msg);

    glCompileShaderARB(mFragmentShader);
    glGetShaderInfoLog(mFragmentShader, sizeof msg, NULL, msg);
    if (strlen(msg) > 0)
        printf("%s: %s\n", pFragmentShaderFileName, msg);

    mShaderProgram = glCreateProgramObjectARB();
    glAttachObjectARB(mShaderProgram, mFragmentShader);
    glAttachObjectARB(mShaderProgram, mVertexShader);
    glBindAttribLocation(mShaderProgram, 0, "position");
    glBindAttribLocation(mShaderProgram, 1, "normal");
    glBindAttribLocation(mShaderProgram, 2, "texture");

    glLinkProgram(mShaderProgram);
    glGetShaderInfoLog(mShaderProgram, sizeof msg, NULL, msg);
    if (strlen(msg) > 0)
        printf("Shader Linking: %s\n", msg);

    mLocations.ModelViewProjectionMatrix = glGetUniformLocation(mShaderProgram,
            "ModelViewProjectionMatrix");
    mLocations.NormalMatrix = glGetUniformLocation(mShaderProgram,
            "NormalMatrix");
    mLocations.LightSourcePosition = glGetUniformLocation(mShaderProgram,
            "LightSourcePosition");
    mLocations.LightSourceDiffuse = glGetUniformLocation(mShaderProgram,
            "LightSourceDiffuse");
    mLocations.MaterialDiffuse = glGetUniformLocation(mShaderProgram,
            "MaterialDiffuse");
#ifdef _DEBUG
    printf("Uniform Locations: %d %d %d %d %d\n",
            mLocations.ModelViewProjectionMatrix,
            mLocations.NormalMatrix,
            mLocations.LightSourcePosition,
            mLocations.LightSourceDiffuse,
            mLocations.MaterialDiffuse);
#endif

}

void Shader::use()
{
    glUseProgramObjectARB(mShaderProgram);
}

void Shader::remove()
{
    glDetachObjectARB(mShaderProgram, mVertexShader);
    glDetachObjectARB(mShaderProgram, mFragmentShader);

    glDeleteObjectARB(mShaderProgram);
}
