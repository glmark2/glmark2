/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010-2011 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
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
#ifndef GLMARK2_MESH_H_
#define GLMARK2_MESH_H_

#include "screen.h"
#include "vec.h"

#include <stdio.h>
#include <math.h>

struct Vertex {
    LibMatrix::vec3 v;
    LibMatrix::vec3 n;
    LibMatrix::vec2 t;
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

    void reset();
    void make_cube();
    void make_torus();
    void render_array(int vertex_loc, int normal_loc, int texcoord_loc);
    void build_vbo();
    void delete_vbo();
    void render_vbo(int vertex_loc, int normal_loc, int texcoord_loc);
};

#endif
