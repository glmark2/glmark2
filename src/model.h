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
#ifndef GLMARK2_MODEL_H_
#define GLMARK2_MODEL_H_

#include "mesh.h"

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

class Polygon
{
public:
    unsigned short mA, mB, mC;
    unsigned short mFaceFlags;
};

// A model as loaded from a 3ds file
class Model
{
public:
    unsigned mPolygonQty;
    unsigned mVertexQty;
    Vertex *mVertex;
    Polygon *mPolygon;
    char mName[20];

    Model();
    ~Model();

    int load_3ds(const char *pFileName);
    void calculate_normals();
    void center();
    void scale(GLfloat pAmount);
    void convert_to_mesh(Mesh *pMesh);
};

#endif
