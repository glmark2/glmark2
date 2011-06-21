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
#include "mesh.h"


Mesh::Mesh() :
    mVertexQty(0), mPolygonQty(0),
    mMode(GL_TRIANGLES), mVertex(0),
    mVBOVertices(0), mVBONormals(0), mVBOTexCoords(0)
{
}

Mesh::~Mesh()
{
    reset();
}

void
Mesh::reset()
{
    delete [] mVertex;

    delete_vbo();

    mPolygonQty = 0;
    mVertexQty = 0;
    mMode = GL_TRIANGLES;
    mVertex = 0;
}

void Mesh::make_cube()
{
    fprintf(stderr, "Warning: %s: Not implemented\n", __FUNCTION__);
}

void Mesh::make_torus()
{
    unsigned wraps_qty = 64;
    unsigned per_wrap_qty = 64;
    float major_radius = 0.8;
    float minor_radius = 0.4;
    unsigned i, j;
    unsigned k = 0;

    LibMatrix::vec3 a, b, c, d, n;

    mMode = GL_TRIANGLES;
    mVertexQty = wraps_qty * per_wrap_qty * 6;
    mVertex = new Vertex[mVertexQty];

    for(i = 0; i < wraps_qty; i++)
        for(j = 0; j < per_wrap_qty; j++)
        {
            float wrap_frac = j / (float)per_wrap_qty;
            float phi = 2 * M_PI * wrap_frac;
            float theta = 2 * M_PI * (i + wrap_frac) / (float)wraps_qty;
            float r = major_radius + minor_radius * (float)cos(phi);
            a.x((float)sin(theta) * r);
            a.y(minor_radius * (float)sin(phi));
            a.z((float)cos(theta) * r);

            theta = 2 * M_PI * (i + wrap_frac + 1) / (float)wraps_qty;
            b.x((float)sin(theta) * r);
            b.y(minor_radius * (float)sin(phi));
            b.z((float)cos(theta) * r);

            wrap_frac = (j + 1) / (float)per_wrap_qty;
            phi = 2 * M_PI * wrap_frac;
            theta = 2 * M_PI * (i + wrap_frac) / (float)wraps_qty;
            r = major_radius + minor_radius * (float)cos(phi);
            c.x((float)sin(theta) * r);
            c.y(minor_radius * (float)sin(phi));
            c.z((float)cos(theta) * r);

            theta = 2 * M_PI * (i + wrap_frac + 1) / (float)wraps_qty;
            d.x((float)sin(theta) * r);
            d.y(minor_radius * (float)sin(phi));
            d.z((float)cos(theta) * r);

            n = LibMatrix::vec3::cross(b - a, c - a);
            n.normalize();
            mVertex[k].n = n;   mVertex[k].v = a;   k++;
            mVertex[k].n = n;   mVertex[k].v = b;   k++;
            mVertex[k].n = n;   mVertex[k].v = c;   k++;
            n = LibMatrix::vec3::cross(c - b, d - b);
            n.normalize();
            mVertex[k].n = n;   mVertex[k].v = b;   k++;
            mVertex[k].n = n;   mVertex[k].v = c;   k++;
            mVertex[k].n = n;   mVertex[k].v = d;   k++;
        }
}

void Mesh::render_array(int vertex_loc, int normal_loc, int texcoord_loc)
{
    // Enable the attributes (texcoord is optional)
    glEnableVertexAttribArray(vertex_loc);
    glEnableVertexAttribArray(normal_loc);
    if (texcoord_loc >= 0)
        glEnableVertexAttribArray(texcoord_loc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE,
                          8 * sizeof(float), mVertex[0].v);
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE,
                          8 * sizeof(float), mVertex[0].n);
    if (texcoord_loc >= 0) {
        glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE,
                              8 * sizeof(float), mVertex[0].t);
    }

    glDrawArrays(GL_TRIANGLES, 0, mVertexQty);

    // Disable the attributes
    glDisableVertexAttribArray(vertex_loc);
    glDisableVertexAttribArray(normal_loc);
    if (texcoord_loc >= 0)
        glDisableVertexAttribArray(texcoord_loc);
}

void Mesh::build_vbo()
{
    float *vertex;
    float *texel;
    float *normal;

    vertex = new float[mVertexQty * 3];
    texel = new float[mVertexQty * 2];
    normal = new float[mVertexQty * 3];

    for(unsigned i = 0; i < mVertexQty; i++)
    {
        vertex[3 * i] = mVertex[i].v.x();
        vertex[3 * i + 1] = mVertex[i].v.y();
        vertex[3 * i + 2] = mVertex[i].v.z();
        texel[2 * i] = mVertex[i].t.x();
        texel[2 * i + 1] = mVertex[i].t.y();
        normal[3 * i] = mVertex[i].n.x();
        normal[3 * i + 1] = mVertex[i].n.y();
        normal[3 * i + 2] = mVertex[i].n.z();
    }

    // Generate And Bind The Vertex Buffer
    glGenBuffers(1, &mVBOVertices);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOVertices);
    // Load The Data
    glBufferData(GL_ARRAY_BUFFER, mVertexQty * 3 * sizeof(float), vertex, GL_STATIC_DRAW);

    // Generate And Bind The normal Buffer
    glGenBuffers(1, &mVBONormals);
    glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
    // Load The Data
    glBufferData(GL_ARRAY_BUFFER, mVertexQty * 3 * sizeof(float), normal, GL_STATIC_DRAW);

    // Generate And Bind The Texture Coordinate Buffer
    glGenBuffers(1, &mVBOTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOTexCoords);
    // Load The Data
    glBufferData(GL_ARRAY_BUFFER, mVertexQty * 2 * sizeof(float), texel, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    delete [] vertex;
    delete [] texel;
    delete [] normal;
}

void
Mesh::delete_vbo()
{
    glDeleteBuffers(1, &mVBOVertices);
    glDeleteBuffers(1, &mVBONormals);
    glDeleteBuffers(1, &mVBOTexCoords);

    mVBOVertices = 0;
    mVBONormals = 0;
    mVBOTexCoords = 0;
}

void Mesh::render_vbo(int vertex_loc, int normal_loc, int texcoord_loc)
{
    // Enable the attributes (texcoord is optional)
    glEnableVertexAttribArray(vertex_loc);
    glEnableVertexAttribArray(normal_loc);
    if (texcoord_loc >= 0)
        glEnableVertexAttribArray(texcoord_loc);

    glBindBuffer(GL_ARRAY_BUFFER, mVBOVertices);
    glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    if (texcoord_loc >= 0) {
        glBindBuffer(GL_ARRAY_BUFFER, mVBOTexCoords);
        glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glDrawArrays(GL_TRIANGLES, 0, mVertexQty);

    // Disable the attributes
    glDisableVertexAttribArray(vertex_loc);
    glDisableVertexAttribArray(normal_loc);
    if (texcoord_loc >= 0)
        glDisableVertexAttribArray(texcoord_loc);
}
