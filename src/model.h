#ifndef _MODEL_H
#define _MODEL_H

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
