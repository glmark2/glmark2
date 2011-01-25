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
#include "model.h"

long filelength(int f)
{
    struct stat buf;
    fstat(f, &buf);
    return(buf.st_size);
}

Model::Model()
{
    mPolygonQty = 0;
    mVertexQty = 0;
    mVertex = 0;        // Set pointer to null
    mPolygon = 0;       // Set pointer to null
}

Model::~Model()
{
    delete [] mVertex;
    delete [] mPolygon;
}

void Model::convert_to_mesh(Mesh *pMesh)
{
#ifdef _DEBUG
    printf("Converting model to mesh...      ");
#endif
    pMesh->mVertexQty = 3 * mPolygonQty;
    pMesh->mPolygonQty = mPolygonQty;
    pMesh->mMode = GL_TRIANGLES;

    pMesh->mVertex = new Vertex[pMesh->mVertexQty];

    for(unsigned i = 0; i < pMesh->mVertexQty; i += 3)
    {
        pMesh->mVertex[i + 0].v = mVertex[mPolygon[i / 3].mA].v;
        pMesh->mVertex[i + 1].v = mVertex[mPolygon[i / 3].mB].v;
        pMesh->mVertex[i + 2].v = mVertex[mPolygon[i / 3].mC].v;

        pMesh->mVertex[i + 0].n = mVertex[mPolygon[i / 3].mA].n;
        pMesh->mVertex[i + 1].n = mVertex[mPolygon[i / 3].mB].n;
        pMesh->mVertex[i + 2].n = mVertex[mPolygon[i / 3].mC].n;

        pMesh->mVertex[i + 0].t = mVertex[mPolygon[i / 3].mA].t;
        pMesh->mVertex[i + 1].t = mVertex[mPolygon[i / 3].mB].t;
        pMesh->mVertex[i + 2].t = mVertex[mPolygon[i / 3].mC].t;
    }

#ifdef _DEBUG
    printf("[ Done ]\n");
#endif
}

void Model::calculate_normals()
{
#ifdef _DEBUG
    printf("Calculating normals for model... ");
#endif
    Vector3f n;

    for(unsigned i = 0; i < mPolygonQty; i++)
    {
        n = normal(mVertex[mPolygon[i].mA].v, mVertex[mPolygon[i].mB].v, mVertex[mPolygon[i].mC].v);
        mVertex[mPolygon[i].mA].n += n;
        mVertex[mPolygon[i].mB].n += n;
        mVertex[mPolygon[i].mC].n += n;
    }

    for(unsigned i = 0; i < mVertexQty; i++)
        mVertex[i].n.normalize();

#ifdef _DEBUG
    printf("[ Done ]\n");
#endif
}

void Model::center()
{
    Vector3f center;
    Vector3f max = mVertex[0].v, min = mVertex[0].v;

    for(unsigned i = 1; i < mVertexQty; i++)
    {
        if(mVertex[i].v.x > max.x) max.x = mVertex[i].v.x;
        if(mVertex[i].v.y > max.y) max.y = mVertex[i].v.y;
        if(mVertex[i].v.z > max.z) max.z = mVertex[i].v.z;

        if(mVertex[i].v.x < min.x) min.x = mVertex[i].v.x;
        if(mVertex[i].v.y < min.y) min.y = mVertex[i].v.y;
        if(mVertex[i].v.z < min.z) min.z = mVertex[i].v.z;
    }

    center = (max + min) / 2.0f;

    for(unsigned i = 0; i < mVertexQty; i++)
        mVertex[i].v -= center;
}

void Model::scale(GLfloat pAmount)
{
    for(unsigned i = 1; i < mVertexQty; i++)
        mVertex[i].v *= pAmount;
}

int Model::load_3ds(const char *pFileName)
{
    int i;
    FILE *l_file;
    unsigned short l_chunk_id;
    unsigned int l_chunk_length;
    unsigned char l_char;
    unsigned short l_qty;
    size_t nread;

#ifdef _DEBUG
    printf("Loading model from 3ds file...   ");
#endif

    if ((l_file=fopen (pFileName, "rb"))== NULL) {
#ifdef _DEBUG
        printf("[ Fail ]\n");
#else
        printf("Could not open 3ds file\n");
#endif
        return 0;
    }

    // Loop to scan the whole file
    while (ftell (l_file) < filelength (fileno (l_file))) {
        // Read the chunk header
        nread = fread (&l_chunk_id, 2, 1, l_file);
        //Read the lenght of the chunk
        nread = fread (&l_chunk_length, 4, 1, l_file);

        switch (l_chunk_id)
        {
            //----------------- MAIN3DS -----------------
            // Description: Main chunk, contains all the other chunks
            // Chunk ID: 4d4d
            // Chunk Lenght: 0 + sub chunks
            //-------------------------------------------
            case 0x4d4d:
                break;

            //----------------- EDIT3DS -----------------
            // Description: 3D Editor chunk, objects layout info
            // Chunk ID: 3d3d (hex)
            // Chunk Lenght: 0 + sub chunks
            //-------------------------------------------
            case 0x3d3d:
                break;

            //--------------- EDIT_OBJECT ---------------
            // Description: Object block, info for each object
            // Chunk ID: 4000 (hex)
            // Chunk Lenght: len(object name) + sub chunks
            //-------------------------------------------
            case 0x4000:
                i = 0;
                do {
                    nread = fread (&l_char, 1, 1, l_file);
                    mName[i] = l_char;
                    i++;
                } while(l_char != '\0' && i<20);
                break;

            //--------------- OBJ_TRIMESH ---------------
            // Description: Triangular mesh, contains chunks for 3d mesh info
            // Chunk ID: 4100 (hex)
            // Chunk Lenght: 0 + sub chunks
            //-------------------------------------------
            case 0x4100:
                break;

            //--------------- TRI_VERTEXL ---------------
            // Description: Vertices list
            // Chunk ID: 4110 (hex)
            // Chunk Lenght: 1 x unsigned short (number of vertices)
            //             + 3 x float (vertex coordinates) x (number of vertices)
            //             + sub chunks
            //-------------------------------------------
            case 0x4110:
                nread = fread (&l_qty, sizeof (unsigned short), 1, l_file);
                mVertexQty = l_qty;
                mVertex = new Vertex[mVertexQty];
                for (i = 0; i < l_qty; i++) {
                    nread = fread (&mVertex[i].v.x, sizeof(float), 1, l_file);
                    nread = fread (&mVertex[i].v.y, sizeof(float), 1, l_file);
                    nread = fread (&mVertex[i].v.z, sizeof(float), 1, l_file);
                }
                break;

            //--------------- TRI_FACEL1 ----------------
            // Description: Polygons (faces) list
            // Chunk ID: 4120 (hex)
            // Chunk Lenght: 1 x unsigned short (number of polygons)
            //             + 3 x unsigned short (polygon points) x (number of polygons)
            //             + sub chunks
            //-------------------------------------------
            case 0x4120:
                nread = fread (&l_qty, sizeof (unsigned short), 1, l_file);
                mPolygonQty = l_qty;
                mPolygon = new Polygon[mPolygonQty];
                for (i = 0; i < l_qty; i++) {
                    nread = fread (&mPolygon[i].mA, sizeof (unsigned short), 1, l_file);
                    nread = fread (&mPolygon[i].mB, sizeof (unsigned short), 1, l_file);
                    nread = fread (&mPolygon[i].mC, sizeof (unsigned short), 1, l_file);
                    nread = fread (&mPolygon[i].mFaceFlags, sizeof (unsigned short), 1, l_file);
                }
                break;

            //------------- TRI_MAPPINGCOORS ------------
            // Description: Vertices list
            // Chunk ID: 4140 (hex)
            // Chunk Lenght: 1 x unsigned short (number of mapping points)
            //             + 2 x float (mapping coordinates) x (number of mapping points)
            //             + sub chunks
            //-------------------------------------------
            case 0x4140:
                nread = fread (&l_qty, sizeof (unsigned short), 1, l_file);
                for (i = 0; i < l_qty; i++) {
                    nread = fread (&mVertex[i].t.u, sizeof (float), 1, l_file);
                    nread = fread (&mVertex[i].t.v, sizeof (float), 1, l_file);
                }
                break;

            //----------- Skip unknow chunks ------------
            //We need to skip all the chunks that currently we don't use
            //We use the chunk lenght information to set the file pointer
            //to the same level next chunk
            //-------------------------------------------
            default:
                fseek(l_file, l_chunk_length - 6, SEEK_CUR);
        }
    }
    fclose(l_file); // Closes the file stream

#ifdef _DEBUG
    printf("[ Success ]\n");
    printf("    Model Information\n");
    printf("    Name:          %s\n", mName);
    printf("    Vertex count:  %d\n", mVertexQty);
    printf("    Polygon count: %d\n", mPolygonQty);
#endif
    return 1;
}
