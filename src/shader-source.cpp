/*
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
 *  Alexandros Frantzis (glmark2)
 */

#include <fstream>

#include "shader-source.h"
#include "log.h"
#include "vec.h"

/** 
 * Loads the contents of a file into a string.
 * 
 * @param filename the name of the file
 * @param str the string to put the contents of the file into
 */
bool
ShaderSource::load_file(const std::string& filename, std::string& str)
{
    using std::ifstream;
    ifstream inputFile(filename.c_str());
    if (!inputFile)
    {
        Log::error("Failed to open \"%s\"\n", filename.c_str());
        return false;
    }

    std::string curLine;
    while (getline(inputFile, curLine))
    {
        str += curLine;
        str += '\n';
    }

    return true;
}


/** 
 * Appends a string to the shader source.
 * 
 * @param str the string to append
 */
void
ShaderSource::append(const std::string &str)
{
    source_ << str;
}

/** 
 * Appends the contents of a file to the shader source.
 * 
 * @param filename the name of the file to append
 */
void
ShaderSource::append_file(const std::string &filename)
{
    std::string source;
    if (load_file(filename, source))
        source_ << source;
}

/** 
 * Replaces a string in the source with another string.
 * 
 * @param remove the string to replace
 * @param insert the string to replace with
 */
void
ShaderSource::replace(const std::string &remove, const std::string &insert)
{
    std::string::size_type pos = 0;
    std::string str(source_.str());

    while ((pos = str.find(remove, pos)) != std::string::npos) {
        str.replace(pos, remove.size(), insert);
        pos++;
    }

    source_.clear();
    source_.str(str);
}

/** 
 * Replaces a string in the source with the contents of a file.
 * 
 * @param remove the string to replace
 * @param filename the name of the file to read from
 */
void
ShaderSource::replace_with_file(const std::string &remove, const std::string &filename)
{
    std::string source;
    if (load_file(filename, source))
        replace(remove, source);
}

/** 
 * Adds a string (usually containing a constant definition) at
 * global (per shader) scope.
 *
 * The string is placed after any default precision qualifiers.
 * 
 * @param str the string to add
 */
void
ShaderSource::add_global(const std::string &str)
{
    std::string::size_type pos = 0;
    std::string source(source_.str());

    /* Find the last precision qualifier */
    pos = source.rfind("precision");

    if (pos != std::string::npos) {
        /* 
         * Find the next #endif line of a preprocessor block that contains
         * the precision qualifier.
         */
        std::string::size_type pos_if = source.find("#if", pos);
        std::string::size_type pos_endif = source.find("#endif", pos);

        if (pos_endif != std::string::npos && pos_endif < pos_if)
            pos = pos_endif;

        /* Go to the next line */
        pos = source.find("\n", pos);
        if (pos != std::string::npos)
            pos++;
    }
    else
        pos = 0;

    source.insert(pos, str);

    source_.clear();
    source_.str(source);
}

/** 
 * Adds a global (per shader) vec3 constant definition.
 *
 * @param name the name of the constant
 * @param v the value of the constant
 */
void
ShaderSource::add_global_const(const std::string &name, const LibMatrix::vec3 &v)
{
    std::stringstream ss;

    ss << "const vec3 " << name << " = vec3(" << std::fixed;
    ss << v.x() << ", " << v.y() << ", " << v.z() << ");" << std::endl;

    add_global(ss.str());
}

/** 
 * Adds a global (per shader) vec4 constant definition.
 *
 * @param name the name of the constant
 * @param v the value of the constant
 */
void
ShaderSource::add_global_const(const std::string &name, const LibMatrix::vec4 &v)
{
    std::stringstream ss;

    ss << "const vec4 " << name << " = vec4(" << std::fixed;
    ss << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ");" << std::endl;

    add_global(ss.str());
}
