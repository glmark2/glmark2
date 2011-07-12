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
#include "options.h"

#include <fstream>
#include <sstream>

#define read_or_fail(file, dst, size) do { \
    file.read(reinterpret_cast<char *>((dst)), (size)); \
    if (file.gcount() < (std::streamsize)(size)) { \
        Log::error("%s: %d: Failed to read %zd bytes from 3ds file (read %zd)\n", \
                   __FUNCTION__, __LINE__, \
                   (size_t)(size), file.gcount()); \
        return false; \
    } \
} while(0);

void
Model::append_object_to_mesh(const Object &object, Mesh &mesh,
                             int p_pos, int n_pos, int t_pos)
{
    size_t face_count = object.faces.size();

    for(size_t i = 0; i < 3 * face_count; i += 3)
    {
        const Face &face = object.faces[i / 3];
        const Vertex &a = object.vertices[face.a];
        const Vertex &b = object.vertices[face.b];
        const Vertex &c = object.vertices[face.c];

        mesh.next_vertex();
        if (p_pos >= 0)
            mesh.set_attrib(p_pos, a.v);
        if (n_pos >= 0)
            mesh.set_attrib(n_pos, a.n);
        if (t_pos >= 0)
            mesh.set_attrib(t_pos, a.t);

        mesh.next_vertex();
        if (p_pos >= 0)
            mesh.set_attrib(p_pos, b.v);
        if (n_pos >= 0)
            mesh.set_attrib(n_pos, b.n);
        if (t_pos >= 0)
            mesh.set_attrib(t_pos, b.t);

        mesh.next_vertex();
        if (p_pos >= 0)
            mesh.set_attrib(p_pos, c.v);
        if (n_pos >= 0)
            mesh.set_attrib(n_pos, c.n);
        if (t_pos >= 0)
            mesh.set_attrib(t_pos, c.t);
    }

}

void
Model::convert_to_mesh(Mesh &mesh)
{
    std::vector<std::pair<AttribType, int> > attribs;

    attribs.push_back(std::pair<AttribType, int>(AttribTypePosition, 3));
    attribs.push_back(std::pair<AttribType, int>(AttribTypeNormal, 3));
    attribs.push_back(std::pair<AttribType, int>(AttribTypeTexcoord, 2));

    convert_to_mesh(mesh, attribs);
}

void
Model::convert_to_mesh(Mesh &mesh,
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

    for (std::vector<Object>::const_iterator iter = objects_.begin();
         iter != objects_.end();
         iter++)
    {
        append_object_to_mesh(*iter, mesh, p_pos, n_pos, t_pos);
    }
}

void
Model::calculate_normals()
{
    LibMatrix::vec3 n;

    for (std::vector<Object>::iterator iter = objects_.begin();
         iter != objects_.end();
         iter++)
    {
        Object &object = *iter;
        size_t face_count = object.faces.size();
        size_t vertex_count = object.vertices.size();

        for(unsigned i = 0; i < face_count; i++)
        {
            const Face &face = object.faces[i];
            Vertex &a = object.vertices[face.a];
            Vertex &b = object.vertices[face.b];
            Vertex &c = object.vertices[face.c];

            n = LibMatrix::vec3::cross(b.v - a.v, c.v - a.v);
            n.normalize();
            a.n += n;
            b.n += n;
            c.n += n;
        }

        for(unsigned i = 0; i < vertex_count; i++)
            object.vertices[i].n.normalize();
    }
}

bool
Model::load_3ds(const std::string &filename)
{
    Object *object(0);

    Log::debug("Loading model from 3ds file '%s'\n", filename.c_str());

    std::fstream input_file(filename.c_str());
    if (!input_file) {
        Log::error("Could not open 3ds file '%s'\n", filename.c_str());
        return false;
    }

    // Loop to scan the whole file
    while (!input_file.eof()) {
        uint16_t chunk_id;
        uint32_t chunk_length;

        // Read the chunk header
        input_file.read(reinterpret_cast<char *>(&chunk_id), 2);
        if (input_file.gcount() == 0) {
            continue;
        }
        else if (input_file.gcount() < 2) {
            Log::error("%s: %d: Failed to read %zd bytes from 3ds file (read %zd)\n",
                       __FUNCTION__, __LINE__, 2, input_file.gcount());
            return false;
        }

        //Read the lenght of the chunk
        read_or_fail(input_file, &chunk_length, 4);

        switch (chunk_id)
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
                {
                std::stringstream ss;
                unsigned char c = 1;

                for (int i = 0; i < 20 && c != '\0'; i++) {
                    read_or_fail(input_file, &c, 1);
                    ss << c;
                }

                objects_.push_back(Object(ss.str()));
                object = &objects_.back();
                }
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
                {
                uint16_t qty;
                read_or_fail(input_file, &qty, sizeof(uint16_t));
                object->vertices.resize(qty);

                for (uint16_t i = 0; i < qty; i++) {
                    float f[3];
                    read_or_fail(input_file, f, sizeof(float) * 3);
                    object->vertices[i].v.x(f[0]);
                    object->vertices[i].v.y(f[1]);
                    object->vertices[i].v.z(f[2]);
                }
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
                {
                uint16_t qty;
                read_or_fail(input_file, &qty, sizeof(uint16_t));
                object->faces.resize(qty);
                for (uint16_t i = 0; i < qty; i++) {
                    read_or_fail(input_file, &object->faces[i].a, sizeof(uint16_t));
                    read_or_fail(input_file, &object->faces[i].b, sizeof(uint16_t));
                    read_or_fail(input_file, &object->faces[i].c, sizeof(uint16_t));
                    read_or_fail(input_file, &object->faces[i].face_flags, sizeof(uint16_t));
                }
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
                {
                uint16_t qty;
                read_or_fail(input_file, &qty, sizeof(uint16_t));
                for (uint16_t i = 0; i < qty; i++) {
                    float f[2];
                    read_or_fail(input_file, f, sizeof(float) * 2);
                    object->vertices[i].t.x(f[0]);
                    object->vertices[i].t.y(f[1]);
                }
                }
                break;

            //----------- Skip unknow chunks ------------
            //We need to skip all the chunks that currently we don't use
            //We use the chunk lenght information to set the file pointer
            //to the same level next chunk
            //-------------------------------------------
            default:
                input_file.seekg(chunk_length - 6, std::ios::cur);
        }
    }

    if (Options::show_debug) {
        for (std::vector<Object>::const_iterator iter = objects_.begin();
             iter != objects_.end();
             iter++)
        {
            Log::debug("    Object name: %s Vertex count: %d Face count: %d\n",
                       iter->name.c_str(), iter->vertices.size(), iter->faces.size());
        }
    }

    return true;
}
