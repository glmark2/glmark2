#ifndef CHARACTERS_H_
#define CHARACTERS_H_

#include <vector>
#include "vec.h"
#include "gl-headers.h"

class PrimitiveState
{
public:
    PrimitiveState(unsigned int type, unsigned int count, unsigned int offset) :
        type_(type),
        count_(count),
        bufferOffset_(offset) {}
    ~PrimitiveState() {}
    void issue() const
    {
        glDrawElements(type_, count_, GL_UNSIGNED_INT, 
            reinterpret_cast<const GLvoid*>(bufferOffset_));
    }
private:
    PrimitiveState();
    unsigned int type_;         // Primitive type (e.g. GL_TRIANGLE_STRIP)
    unsigned int count_;        // Number of primitives
    unsigned int bufferOffset_; // Offset into the element array buffer
};

struct Character
{
    void draw()
    {
        //glBindVertexArray(vertexArray_);
        glBindBuffer(GL_ARRAY_BUFFER, bufferObjects_[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects_[1]);
        for (std::vector<PrimitiveState>::const_iterator primIt = primVec_.begin();
             primIt != primVec_.end();
             primIt++)
        {
            primIt->issue();
        }
        //glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    void init(int vertexAttribIndex) 
    {
        vertexIndex_ = vertexAttribIndex;

        // Create and bind a vertex array object to hold our buffer and 
        // attribute state.
        //glGenVertexArrays(1, &vertexArray_);
        //glBindVertexArray(vertexArray_);

        // We need 2 buffers for our work here.  One for the vertex data.
        // and one for the index data.
        glGenBuffers(2, &bufferObjects_[0]);

        // First, setup the vertex data by binding the first buffer object, 
        // allocating its data store, and filling it in with our vertex data.
        glBindBuffer(GL_ARRAY_BUFFER, bufferObjects_[0]);
        glBufferData(GL_ARRAY_BUFFER, vertexData_.size() * sizeof(LibMatrix::vec2), 
            vertexData_.data(), GL_STATIC_DRAW);

        // Finally, setup the pointer to our vertex data and enable this
        // attribute array.
        glVertexAttribPointer(vertexIndex_, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertexIndex_);

        // Now repeat for our index data.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects_[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            indexData_.size() * sizeof(unsigned int), indexData_.data(), 
            GL_STATIC_DRAW);

        // Unbind our vertex array object so that it's state isn't affected by
        // other objects.
        //glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    ~Character()
    {
        glDeleteBuffers(2, &bufferObjects_[0]);
        //glDeleteVertexArrays(1, &vertexArray_);
    }
    Character() :
        vertexIndex_(0),
        vertexArray_(0) {}
    unsigned int bufferObjects_[2];
    std::vector<LibMatrix::vec2> vertexData_;
    std::vector<unsigned int> indexData_;
    int vertexIndex_;
    unsigned int vertexArray_;
    std::vector<PrimitiveState> primVec_;
};

struct LetterI : Character
{
    LetterI();
};

struct LetterD : Character
{
    LetterD();
};

struct LetterE : Character
{
    LetterE();
};

struct LetterA : Character
{
    LetterA();
};

struct LetterS : Character
{
    LetterS();
};

struct LetterN : Character
{
    LetterN();
};

struct LetterM : Character
{
    LetterM();
};

struct LetterO : Character
{
    LetterO();
};

struct LetterT : Character
{
    LetterT();
};

#endif // CHARACTERS_H_
