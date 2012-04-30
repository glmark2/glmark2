#ifndef LAMP_H_
#define LAMP_H_

#include <string>
#include <vector>
#include "vec.h"
#include "stack.h"
#include "gl-headers.h"
#include "program.h"

class Lamp
{
public:
    Lamp();
    ~Lamp();

    void init();
    bool valid() const { return valid_; }
    void draw(LibMatrix::Stack4& modelview, LibMatrix::Stack4& projection,
              const LibMatrix::vec4* lightPositions);
private:    
    Program litProgram_;
    Program unlitProgram_;
    std::string litVertexShader_;
    std::string litFragmentShader_;
    std::string unlitVertexShader_;
    std::string unlitFragmentShader_;
    static const std::string modelviewName_;
    static const std::string projectionName_;
    static const std::string light0PositionName_;
    static const std::string light1PositionName_;
    static const std::string light2PositionName_;
    static const std::string vertexAttribName_;
    static const std::string normalAttribName_;
    static const std::string normalMatrixName_;
    std::vector<LibMatrix::vec3> vertexData_;
    std::vector<unsigned int> indexData_;
    unsigned int bufferObjects_[2];
    bool valid_;
};

#endif // LAMP_H_
