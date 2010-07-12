#ifndef _MESH_H
#define _MESH_H

#include "screen.h"
#include "vector.h"

#include <stdio.h>
#include <math.h>

class Texel
{
public:
    GLfloat u, v;
    
    Texel();
    Texel(GLfloat pU, GLfloat pV);
};

class Vertex
{
public:
    Vector3f v;
    Vector3f n;
    Texel t;
};

// Data for a mesh to be rendered by vertex arrays' or vbos' has 3 verticies per
// polygon and no polygonal data
class Mesh
{
public:
    unsigned mVertexQty;         // Quantity of Verticies
    unsigned mPolygonQty;        // Quantity of polygons, not really needed
    GLenum mMode;           // Polygon mode, eg GL_QUADS, GL_TRIANGLES etc...
    Vertex *mVertex;        // Storage for the verticies

    GLuint mVBOVertices;    // Vertex VBO name
    GLuint mVBONormals;     // Texture coordinate VBO name
    GLuint mVBOTexCoords;   // Texture coordinate VBO name
    
    Mesh();                 // Default Constructor, should set pointers to null
    ~Mesh();
    
    void make_cube();
    void make_torus();
    void render_array();
    void build_vbo();
    void render_vbo();
};

#endif
