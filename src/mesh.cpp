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
#include "shader.h"

Texel::Texel()
{
    u = 0;  v = 0;
}

Texel::Texel(GLfloat pU, GLfloat pV)
{
    u = pU; v = pV;
}

Mesh::Mesh()
{
    mPolygonQty = 0;
    mVertexQty = 0;
    mMode = GL_TRIANGLES;
    mVertex = 0;
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

    Vector3f a, b, c, d, n;

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
            a.x = (float)sin(theta) * r;
            a.y = minor_radius * (float)sin(phi);
            a.z = (float)cos(theta) * r;

            theta = 2 * M_PI * (i + wrap_frac + 1) / (float)wraps_qty;
            b.x = (float)sin(theta) * r;
            b.y = minor_radius * (float)sin(phi);
            b.z = (float)cos(theta) * r;

            wrap_frac = (j + 1) / (float)per_wrap_qty;
            phi = 2 * M_PI * wrap_frac;
            theta = 2 * M_PI * (i + wrap_frac) / (float)wraps_qty;
            r = major_radius + minor_radius * (float)cos(phi);
            c.x = (float)sin(theta) * r;
            c.y = minor_radius * (float)sin(phi);
            c.z = (float)cos(theta) * r;

            theta = 2 * M_PI * (i + wrap_frac + 1) / (float)wraps_qty;
            d.x = (float)sin(theta) * r;
            d.y = minor_radius * (float)sin(phi);
            d.z = (float)cos(theta) * r;

            n = normal(a, b, c);
            mVertex[k].n = n;   mVertex[k].v = a;   k++;
            mVertex[k].n = n;   mVertex[k].v = b;   k++;
            mVertex[k].n = n;   mVertex[k].v = c;   k++;
            n = normal(a, b, c);
            mVertex[k].n = n;   mVertex[k].v = b;   k++;
            mVertex[k].n = n;   mVertex[k].v = c;   k++;
            mVertex[k].n = n;   mVertex[k].v = d;   k++;
        }
}

void Mesh::render_array()
{
    // Enable the attributes
    glEnableVertexAttribArray(Shader::VertexAttribLocation);
    glEnableVertexAttribArray(Shader::NormalAttribLocation);
    glEnableVertexAttribArray(Shader::TexCoordAttribLocation);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(Shader::VertexAttribLocation, 3, GL_FLOAT,
                          GL_FALSE, sizeof(Vertex), &mVertex[0].v.x);
    glVertexAttribPointer(Shader::NormalAttribLocation, 3, GL_FLOAT,
                          GL_FALSE, sizeof(Vertex), &mVertex[0].n.x);
    glVertexAttribPointer(Shader::TexCoordAttribLocation, 2, GL_FLOAT,
                          GL_FALSE, sizeof(Vertex), &mVertex[0].t.u);

    glDrawArrays(GL_TRIANGLES, 0, mVertexQty);

    // Disable the attributes
    glDisableVertexAttribArray(Shader::TexCoordAttribLocation);
    glDisableVertexAttribArray(Shader::NormalAttribLocation);
    glDisableVertexAttribArray(Shader::VertexAttribLocation);
}

void Mesh::build_vbo()
{
#ifdef _DEBUG
    printf("Building vbo for mesh...         ");
#endif

    Vector3f *vertex;
    Texel *texel;
    Vector3f *normal;

    vertex = new Vector3f[mVertexQty];
    texel = new Texel[mVertexQty];
    normal = new Vector3f[mVertexQty];

    for(unsigned i = 0; i < mVertexQty; i++)
    {
        vertex[i] = mVertex[i].v;
        texel[i] = mVertex[i].t;
        normal[i] = mVertex[i].n;
    }

    // Generate And Bind The Vertex Buffer
    glGenBuffers(1, &mVBOVertices);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOVertices);
    // Load The Data
    glBufferData(GL_ARRAY_BUFFER, mVertexQty * sizeof(Vector3f), vertex, GL_STATIC_DRAW);

    // Generate And Bind The normal Buffer
    glGenBuffers(1, &mVBONormals);
    glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
    // Load The Data
    glBufferData(GL_ARRAY_BUFFER, mVertexQty * sizeof(Vector3f), normal, GL_STATIC_DRAW);

    // Generate And Bind The Texture Coordinate Buffer
    glGenBuffers(1, &mVBOTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOTexCoords);
    // Load The Data
    glBufferData(GL_ARRAY_BUFFER, mVertexQty * sizeof(Texel), texel, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    delete [] vertex;
    delete [] texel;
    delete [] normal;
#ifdef _DEBUG
    printf("[ Done ]\n");
#endif
}

void
Mesh::delete_vbo()
{
    glDeleteBuffers(1, &mVBOVertices);
    glDeleteBuffers(1, &mVBONormals);
    glDeleteBuffers(1, &mVBOTexCoords);
}

void Mesh::render_vbo()
{
    // Enable the attributes
    glEnableVertexAttribArray(Shader::VertexAttribLocation);
    glEnableVertexAttribArray(Shader::NormalAttribLocation);
    glEnableVertexAttribArray(Shader::TexCoordAttribLocation);

    glBindBuffer(GL_ARRAY_BUFFER, mVBOVertices);
    glVertexAttribPointer(Shader::VertexAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
    glVertexAttribPointer(Shader::NormalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOTexCoords);
    glVertexAttribPointer(Shader::TexCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, mVertexQty);

    // Disable the attributes
    glDisableVertexAttribArray(Shader::TexCoordAttribLocation);
    glDisableVertexAttribArray(Shader::NormalAttribLocation);
    glDisableVertexAttribArray(Shader::VertexAttribLocation);
}
