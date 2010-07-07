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
	glCompileShaderARB(mFragmentShader);

	mShaderProgram = glCreateProgramObjectARB();
	glAttachObjectARB(mShaderProgram, mFragmentShader);
	glAttachObjectARB(mShaderProgram, mVertexShader);

	glLinkProgram(mShaderProgram);
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
