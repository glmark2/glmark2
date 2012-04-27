#include "characters.h"

using LibMatrix::vec2;

LetterD::LetterD()
{
    // Vertex data...
    vertexData_.push_back(vec2(4.714579, 9.987679));
    vertexData_.push_back(vec2(2.841889, 9.429158));
    vertexData_.push_back(vec2(2.825462, 9.166325));
    vertexData_.push_back(vec2(1.856263, 8.722793));
    vertexData_.push_back(vec2(2.004107, 8.000000));
    vertexData_.push_back(vec2(0.969199, 7.605750));
    vertexData_.push_back(vec2(1.494866, 6.636550));
    vertexData_.push_back(vec2(0.607803, 6.028748));
    vertexData_.push_back(vec2(1.527721, 4.960986));
    vertexData_.push_back(vec2(0.772074, 4.254620));
    vertexData_.push_back(vec2(1.774127, 4.139630));
    vertexData_.push_back(vec2(1.445585, 3.186858));
    vertexData_.push_back(vec2(2.266940, 3.843942));
    vertexData_.push_back(vec2(2.250513, 3.022587));
    vertexData_.push_back(vec2(2.776181, 3.843942));
    vertexData_.push_back(vec2(3.137577, 3.383984));
    vertexData_.push_back(vec2(3.351129, 4.008214));
    vertexData_.push_back(vec2(3.909651, 4.451746));
    vertexData_.push_back(vec2(4.090349, 4.960986));
    vertexData_.push_back(vec2(4.862423, 5.946612));
    vertexData_.push_back(vec2(4.763860, 6.652977));
    vertexData_.push_back(vec2(5.388090, 7.572895));
    vertexData_.push_back(vec2(4.862423, 8.492813));
    vertexData_.push_back(vec2(5.618070, 9.921971));
    vertexData_.push_back(vec2(4.698152, 10.940452));
    vertexData_.push_back(vec2(5.338809, 12.303902));
    vertexData_.push_back(vec2(4.238193, 12.960985));
    vertexData_.push_back(vec2(4.451746, 14.554415));
    vertexData_.push_back(vec2(3.581109, 14.291581));
    vertexData_.push_back(vec2(3.613963, 15.342916));
    vertexData_.push_back(vec2(2.677618, 15.145790));
    vertexData_.push_back(vec2(2.480493, 15.540041));
    vertexData_.push_back(vec2(2.036961, 15.211499));
    vertexData_.push_back(vec2(1.281314, 15.112936));

    // Index data...
    indexData_.push_back(0);
    indexData_.push_back(1);
    indexData_.push_back(2);
    indexData_.push_back(3);
    indexData_.push_back(4);
    indexData_.push_back(5);
    indexData_.push_back(6);
    indexData_.push_back(7);
    indexData_.push_back(8);
    indexData_.push_back(9);
    indexData_.push_back(10);
    indexData_.push_back(11);
    indexData_.push_back(12);
    indexData_.push_back(13);
    indexData_.push_back(14);
    indexData_.push_back(15);
    indexData_.push_back(16);
    indexData_.push_back(17);
    indexData_.push_back(18);
    indexData_.push_back(19);
    indexData_.push_back(20);
    indexData_.push_back(21);
    indexData_.push_back(22);
    indexData_.push_back(23);
    indexData_.push_back(24);
    indexData_.push_back(25);
    indexData_.push_back(26);
    indexData_.push_back(27);
    indexData_.push_back(28);
    indexData_.push_back(29);
    indexData_.push_back(30);
    indexData_.push_back(31);
    indexData_.push_back(32);
    indexData_.push_back(33);
    indexData_.push_back(0);
    indexData_.push_back(2);
    indexData_.push_back(4);
    indexData_.push_back(6);
    indexData_.push_back(8);
    indexData_.push_back(10);
    indexData_.push_back(12);
    indexData_.push_back(14);
    indexData_.push_back(16);
    indexData_.push_back(18);
    indexData_.push_back(20);
    indexData_.push_back(22);
    indexData_.push_back(24);
    indexData_.push_back(26);
    indexData_.push_back(28);
    indexData_.push_back(30);
    indexData_.push_back(32);
    indexData_.push_back(33);
    indexData_.push_back(31);
    indexData_.push_back(29);
    indexData_.push_back(27);
    indexData_.push_back(25);
    indexData_.push_back(23);
    indexData_.push_back(21);
    indexData_.push_back(19);
    indexData_.push_back(17);
    indexData_.push_back(15);
    indexData_.push_back(13);
    indexData_.push_back(11);
    indexData_.push_back(9);
    indexData_.push_back(7);
    indexData_.push_back(5);
    indexData_.push_back(3);
    indexData_.push_back(1);

    // Primitive state so that the draw call can issue the primitives we want.
    unsigned int curOffset(0);
    primVec_.push_back(PrimitiveState(GL_TRIANGLE_STRIP, 34, curOffset));
    curOffset += (34 * sizeof(unsigned int));
    primVec_.push_back(PrimitiveState(GL_LINE_STRIP, 34, curOffset));
}
