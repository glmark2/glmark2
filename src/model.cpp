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
#include "vec.h"
#include "log.h"

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

void Model::convert_to_mesh(Mesh &mesh)
{
    std::vector<std::pair<AttribType, int> > attribs;

    attribs.push_back(std::pair<AttribType, int>(AttribTypePosition, 3));
    attribs.push_back(std::pair<AttribType, int>(AttribTypeNormal, 3));
    attribs.push_back(std::pair<AttribType, int>(AttribTypeTexcoord, 2));

    convert_to_mesh(mesh, attribs);
}

void Model::convert_to_mesh(Mesh &mesh,
                            const std::vector<std::pair<AttribType, int> > &attribs)
{
    std::vector<int> format;
    int p_pos = -1;
    int n_pos = -1;
    int t_pos = -1;

    mesh.reset();

    for (std::vector<std::pair<AttribType, int> >::const_iterator ai = attribs.begin();
         ai != attribs.end();
         ai++)
    {
        format.push_back(ai->second);
        if (ai->first == AttribTypePosition)
            p_pos = ai - attribs.begin();
        else if (ai->first == AttribTypeNormal)
            n_pos = ai - attribs.begin();
        else if (ai->first == AttribTypeTexcoord)
            t_pos = ai - attribs.begin();
    }

    mesh.set_vertex_format(format);

    for(unsigned i = 0; i < 3 * mPolygonQty; i += 3)
    {
        mesh.next_vertex();
        if (p_pos >= 0)
            mesh.set_attrib(p_pos, mVertex[mPolygon[i / 3].mA].v);
        if (n_pos >= 0)
            mesh.set_attrib(n_pos, mVertex[mPolygon[i / 3].mA].n);
        if (t_pos >= 0)
            mesh.set_attrib(t_pos, mVertex[mPolygon[i / 3].mA].t);

        mesh.next_vertex();
        if (p_pos >= 0)
            mesh.set_attrib(p_pos, mVertex[mPolygon[i / 3].mB].v);
        if (n_pos >= 0)
            mesh.set_attrib(n_pos, mVertex[mPolygon[i / 3].mB].n);
        if (t_pos >= 0)
            mesh.set_attrib(t_pos, mVertex[mPolygon[i / 3].mB].t);

        mesh.next_vertex();
        if (p_pos >= 0)
            mesh.set_attrib(p_pos, mVertex[mPolygon[i / 3].mC].v);
        if (n_pos >= 0)
            mesh.set_attrib(n_pos, mVertex[mPolygon[i / 3].mC].n);
        if (t_pos >= 0)
            mesh.set_attrib(t_pos, mVertex[mPolygon[i / 3].mC].t);
    }
}

void Model::calculate_normals()
{
    LibMatrix::vec3 n;

    for(unsigned i = 0; i < mPolygonQty; i++)
    {
        n = LibMatrix::vec3::cross(mVertex[mPolygon[i].mB].v - mVertex[mPolygon[i].mA].v,
                                   mVertex[mPolygon[i].mC].v - mVertex[mPolygon[i].mA].v);
        n.normalize();
        mVertex[mPolygon[i].mA].n += n;
        mVertex[mPolygon[i].mB].n += n;
        mVertex[mPolygon[i].mC].n += n;
    }

    for(unsigned i = 0; i < mVertexQty; i++)
        mVertex[i].n.normalize();
}

void Model::center()
{
    LibMatrix::vec3 max(mVertex[0].v);
    LibMatrix::vec3 min(mVertex[0].v);

    for(unsigned i = 1; i < mVertexQty; i++)
    {
        if(mVertex[i].v.x() > max.x()) max.x(mVertex[i].v.x());
        if(mVertex[i].v.y() > max.y()) max.y(mVertex[i].v.y());
        if(mVertex[i].v.z() > max.z()) max.z(mVertex[i].v.z());

        if(mVertex[i].v.x() < min.x()) min.x(mVertex[i].v.x());
        if(mVertex[i].v.y() < min.y()) min.y(mVertex[i].v.y());
        if(mVertex[i].v.z() < min.z()) min.z(mVertex[i].v.z());
    }

    LibMatrix::vec3 center(max + min);
    center /= 2.0f;

    for(unsigned i = 0; i < mVertexQty; i++)
        mVertex[i].v -= center;
}

void Model::scale(GLfloat pAmount)
{
    for(unsigned i = 1; i < mVertexQty; i++)
        mVertex[i].v *= pAmount;
}

#define fread_or_fail(a, b, c, d) do { \
    size_t nread_; \
    nread_ = fread((a), (b), (c), (d));\
    if (nread_ < (c)) { \
        Log::error("Failed to read %zd bytes from 3ds file (read %zd)\n", \
                   (size_t)((c) * (b)), nread_ * (b)); \
        return 0; \
    } \
} while(0);

int Model::load_3ds(const char *pFileName)
{
    int i;
    FILE *l_file;
    unsigned short l_chunk_id;
    unsigned int l_chunk_length;
    unsigned char l_char;
    unsigned short l_qty;

    Log::debug("Loading model from 3ds file '%s'\n", pFileName);

    if ((l_file = fopen (pFileName, "rb")) == NULL) {
        Log::error("Could not open 3ds file '%s'\n", pFileName);
        return 0;
    }

    // Loop to scan the whole file
    while (ftell (l_file) < filelength (fileno (l_file))) {
        // Read the chunk header
        fread_or_fail (&l_chunk_id, 2, 1, l_file);
        //Read the lenght of the chunk
        fread_or_fail (&l_chunk_length, 4, 1, l_file);

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
                    fread_or_fail (&l_char, 1, 1, l_file);
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
                fread_or_fail (&l_qty, sizeof (unsigned short), 1, l_file);
                mVertexQty = l_qty;
                mVertex = new Vertex[mVertexQty];
                for (i = 0; i < l_qty; i++) {
                    float f[3];
                    fread_or_fail (f, sizeof(float), 3, l_file);
                    mVertex[i].v.x(f[0]);
                    mVertex[i].v.y(f[1]);
                    mVertex[i].v.z(f[2]);
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
                fread_or_fail (&l_qty, sizeof (unsigned short), 1, l_file);
                mPolygonQty = l_qty;
                mPolygon = new Polygon[mPolygonQty];
                for (i = 0; i < l_qty; i++) {
                    fread_or_fail (&mPolygon[i].mA, sizeof (unsigned short), 1, l_file);
                    fread_or_fail (&mPolygon[i].mB, sizeof (unsigned short), 1, l_file);
                    fread_or_fail (&mPolygon[i].mC, sizeof (unsigned short), 1, l_file);
                    fread_or_fail (&mPolygon[i].mFaceFlags, sizeof (unsigned short), 1, l_file);
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
                fread_or_fail (&l_qty, sizeof (unsigned short), 1, l_file);
                for (i = 0; i < l_qty; i++) {
                    float f[2];
                    fread_or_fail (f, sizeof(float), 2, l_file);
                    mVertex[i].t.x(f[0]);
                    mVertex[i].t.y(f[1]);
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

    Log::debug("    Model Information\n"
               "    Name:          %s\n"
               "    Vertex count:  %d\n"
               "    Polygon count: %d\n",
               mName, mVertexQty, mPolygonQty);
    return 1;
}
