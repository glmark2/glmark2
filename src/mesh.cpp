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
#include "log.h"


Mesh::Mesh() :
    vertex_size_(0)
{
}

Mesh::~Mesh()
{
    reset();
}

/*
 * Sets the vertex format for this mesh.
 *
 * The format consists of a vector of integers, each
 * specifying the size in floats of each vertex attribute.
 *
 * e.g. {4, 3, 2} => 3 attributes vec4, vec3, vec2
 */
void
Mesh::set_vertex_format(const std::vector<int> &format)
{
    int pos = 0;
    vertex_format_.clear();

    for (std::vector<int>::const_iterator iter = format.begin();
         iter != format.end();
         iter++)
    {
        int n = *iter;
        vertex_format_.push_back(std::pair<int,int>(n, pos));

        pos += n;
    }

    vertex_size_ = pos;
}

/*
 * Sets the attribute locations.
 *
 * These are the locations used in glEnableVertexAttribArray()
 * and other related functions.
 */
void
Mesh::set_attrib_locations(const std::vector<int> &locations)
{
    if (locations.size() != vertex_format_.size())
        Log::error("Trying to set attribute locations using wrong size\n");
    attrib_locations_ = locations;
}


bool
Mesh::check_attrib(int pos, int size)
{
    if (pos > (int)vertex_format_.size()) {
        Log::error("Trying to set non-existent attribute\n");
        return false;
    }

    if (vertex_format_[pos].first != size) {
        Log::error("Trying to set attribute with value of invalid type\n");
        return false;
    }

    return true;
}


std::vector<float> &
Mesh::ensure_vertex()
{
    if (vertices_.empty())
        next_vertex();

    return vertices_.back();
}

/*
 * Sets the value of an attribute in the current vertex.
 *
 * The pos parameter refers to the position of the attribute
 * as specified indirectly when setting the format using
 * set_vertex_format(). e.g. 0 = first attribute, 1 = second
 * etc
 */
void
Mesh::set_attrib(int pos, const LibMatrix::vec2 &v, std::vector<float> *vertex)
{
    if (!check_attrib(pos, 2))
        return;

    std::vector<float> &vtx = !vertex ? ensure_vertex() : *vertex;

    int offset = vertex_format_[pos].second;

    vtx[offset] = v.x();
    vtx[offset + 1] = v.y();
}

void
Mesh::set_attrib(int pos, const LibMatrix::vec3 &v, std::vector<float> *vertex)
{
    if (!check_attrib(pos, 3))
        return;

    std::vector<float> &vtx = !vertex ? ensure_vertex() : *vertex;

    int offset = vertex_format_[pos].second;

    vtx[offset] = v.x();
    vtx[offset + 1] = v.y();
    vtx[offset + 2] = v.z();
}

void
Mesh::set_attrib(int pos, const LibMatrix::vec4 &v, std::vector<float> *vertex)
{
    if (!check_attrib(pos, 4))
        return;

    std::vector<float> &vtx = !vertex ? ensure_vertex() : *vertex;

    int offset = vertex_format_[pos].second;

    vtx[offset] = v.x();
    vtx[offset + 1] = v.y();
    vtx[offset + 2] = v.z();
    vtx[offset + 3] = v.w();
}

/*
 * Adds a new vertex to the list and makes it current.
 */
void
Mesh::next_vertex()
{
    vertices_.push_back(std::vector<float>(vertex_size_));
}

void
Mesh::reset()
{
    delete_array();
    delete_vbo();

    vertices_.clear();
    vertex_format_.clear();
    attrib_locations_.clear();
    attrib_data_ptr_.clear();
    vertex_size_ = 0;
    vertex_stride_ = 0;
}

void
Mesh::build_array(bool interleaved)
{
    int nvertices = vertices_.size();

    if (!interleaved) {
        /* Create an array for each attribute */
        for (std::vector<std::pair<int, int> >::const_iterator ai = vertex_format_.begin();
             ai != vertex_format_.end();
             ai++)
        {
            float *array = new float[nvertices * ai->first];
            float *cur = array;

            /* Fill in the array */
            for (std::vector<std::vector<float> >::const_iterator vi = vertices_.begin();
                    vi != vertices_.end();
                    vi++)
            {
                for (int i = 0; i < ai->first; i++)
                    *cur++ = (*vi)[ai->second + i];
            }

            vertex_arrays_.push_back(array);
            attrib_data_ptr_.push_back(array);
        }
        vertex_stride_ = 0;
    }
    else {
        float *array = new float[nvertices * vertex_size_];
        float *cur = array;

        for (std::vector<std::vector<float> >::const_iterator vi = vertices_.begin();
             vi != vertices_.end();
             vi++)
        {
            /* Fill in the array */
            for (int i = 0; i < vertex_size_; i++)
                *cur++ = (*vi)[i];
        }

        for (size_t i = 0; i < vertex_format_.size(); i++)
            attrib_data_ptr_.push_back(array + vertex_format_[i].second);

        vertex_arrays_.push_back(array);
        vertex_stride_ = vertex_size_ * sizeof(float);
    }
}

void
Mesh::build_vbo(bool interleave)
{
    delete_array();
    build_array(interleave);

    int nvertices = vertices_.size();

    attrib_data_ptr_.clear();

    if (!interleave) {
        /* Create a vbo for each attribute */
        for (std::vector<std::pair<int, int> >::const_iterator ai = vertex_format_.begin();
             ai != vertex_format_.end();
             ai++)
        {
            float *data = vertex_arrays_[ai - vertex_format_.begin()];
            GLuint vbo;

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, nvertices * ai->first * sizeof(float),
                         data, GL_STATIC_DRAW);

            vbos_.push_back(vbo);
            attrib_data_ptr_.push_back(0);
        }

        vertex_stride_ = 0;
    }
    else {
        GLuint vbo;
        /* Create a single vbo to store all attribute data */
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, nvertices * vertex_size_ * sizeof(float),
                     vertex_arrays_[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        for (size_t i = 0; i < vertex_format_.size(); i++) {
            attrib_data_ptr_.push_back(reinterpret_cast<float *>(sizeof(float) * vertex_format_[i].second));
            vbos_.push_back(vbo);
        }
        vertex_stride_ = vertex_size_ * sizeof(float);
    }

    delete_array();
}

void
Mesh::delete_array()
{
    for (size_t i = 0; i < vertex_arrays_.size(); i++) {
        delete [] vertex_arrays_[i];
    }

    vertex_arrays_.clear();
}

void
Mesh::delete_vbo()
{
    for (size_t i = 0; i < vbos_.size(); i++) {
        GLuint vbo = vbos_[i];
        glDeleteBuffers(1, &vbo);
    }

    vbos_.clear();
}


void
Mesh::render_array()
{
    for (size_t i = 0; i < vertex_format_.size(); i++) {
        glEnableVertexAttribArray(attrib_locations_[i]);
        glVertexAttribPointer(attrib_locations_[i], vertex_format_[i].first,
                              GL_FLOAT, GL_FALSE, vertex_stride_,
                              attrib_data_ptr_[i]);
    }

    glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

    for (size_t i = 0; i < vertex_format_.size(); i++) {
        glDisableVertexAttribArray(attrib_locations_[i]);
    }
}

void
Mesh::render_vbo()
{
    for (size_t i = 0; i < vertex_format_.size(); i++) {
        glEnableVertexAttribArray(attrib_locations_[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos_[i]);
        glVertexAttribPointer(attrib_locations_[i], vertex_format_[i].first,
                              GL_FLOAT, GL_FALSE, vertex_stride_,
                              attrib_data_ptr_[i]);
    }

    glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

    for (size_t i = 0; i < vertex_format_.size(); i++) {
        glDisableVertexAttribArray(attrib_locations_[i]);
    }
}


