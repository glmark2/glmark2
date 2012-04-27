#ifndef TABLE_H_
#define TABLE_H_

#include <string>
#include <vector>
#include "characters.h"
#include "program.h"
#include "stack.h"

class Table
{
public:
    Table();
    ~Table();

    void init();
    bool valid() const { return valid_; }
    void draw(LibMatrix::Stack4& modelview, LibMatrix::Stack4& projection, 
              const LibMatrix::vec3& lightPosition, const LibMatrix::vec3& logoPosition,
              const float& currentTime, float& paperAlpha_out);
    void drawUnder(LibMatrix::Stack4& modelview, LibMatrix::Stack4& projection);

private:
    // Text
    LetterI i_;
    LetterD d_;
    LetterE e_;
    LetterA a_;
    LetterS s_;
    LetterN n_;
    LetterM m_;
    LetterO o_;
    LetterT t_;
    Program tableProgram_;
    Program paperProgram_;
    Program textProgram_;
    Program underProgram_;
    std::string tableVertexShader_;
    std::string tableFragmentShader_;
    std::string paperVertexShader_;
    std::string paperFragmentShader_;
    std::string textVertexShader_;
    std::string textFragmentShader_;
    std::string underVertexShader_;
    std::string underFragmentShader_;
    static const std::string modelviewName_;
    static const std::string projectionName_;
    static const std::string lightPositionName_;
    static const std::string logoDirectionName_;
    static const std::string curTimeName_;
    static const std::string vertexAttribName_;
    static const unsigned int TABLERES_;
    std::vector<LibMatrix::vec3> tableVertices_;
    static const LibMatrix::vec3 paperVertices_[4];
    struct VertexDataMap
    {
        unsigned int tvOffset;
        unsigned int tvSize;
        unsigned int pvOffset;
        unsigned int pvSize;
        unsigned int totalSize;
    } dataMap_;
    unsigned int bufferObjects_[2];
    std::vector<unsigned int> indexData_;
    int tableVertexIndex_;
    int paperVertexIndex_;
    int textVertexIndex_;
    int underVertexIndex_;
    bool valid_;
};

#endif // TABLE_H_
