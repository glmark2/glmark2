/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010 Linaro
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Ben Smith (original glmark benchmark)
 *  Alexandros Frantzis (glmark2)
 */
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
