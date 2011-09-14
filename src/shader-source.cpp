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

#include <istream>
#include <memory>

#include "shader-source.h"
#include "log.h"
#include "vec.h"
#include "util.h"

ShaderSource::Precision ShaderSource::default_vertex_precision_;
ShaderSource::Precision ShaderSource::default_fragment_precision_;

/** 
 * Loads the contents of a file into a string.
 * 
 * @param filename the name of the file
 * @param str the string to put the contents of the file into
 */
bool
ShaderSource::load_file(const std::string& filename, std::string& str)
{
    std::auto_ptr<std::istream> is_ptr(Util::get_resource(filename));
    std::istream& inputFile(*is_ptr);

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
 * Adds a string (usually containing a constant definition) at
 * global (per shader) scope.
 *
 * The string is placed after any default precision qualifiers.
 * 
 * @param function the function to add the string into
 * @param str the string to add
 */
void
ShaderSource::add_local(const std::string &str, const std::string &function)
{
    std::string::size_type pos = 0;
    std::string source(source_.str());

    /* Find the function */
    pos = source.find(function);
    pos = source.find('{', pos);

    /* Go to the next line */
    pos = source.find("\n", pos);
    if (pos != std::string::npos)
        pos++;

    source.insert(pos, str);

    source_.clear();
    source_.str(source);
}

/** 
 * Adds a string (usually containing a constant definition) to a shader source
 *
 * If the function parameter is empty, the string will be added to global
 * scope, after any precision definitions.
 *
 * @param str the string to add
 * @param function if not empty, the function to add the string into
 */
void
ShaderSource::add(const std::string &str, const std::string &function)
{
    if (!function.empty())
        add_local(str, function);
    else
        add_global(str);
}

/** 
 * Adds a float constant definition.
 *
 * @param name the name of the constant
 * @param f the value of the constant
 * @param function if not empty, the function to put the definition in
 */
void
ShaderSource::add_const(const std::string &name, float f,
                        const std::string &function)
{
    std::stringstream ss;

    ss << "const float " << name << " = " << std::fixed << f << ";" << std::endl;

    add(ss.str(), function);
}

/** 
 * Adds a float array constant definition.
 *
 * Note that various GLSL versions (including ES) don't support
 * array constants.
 *
 * @param name the name of the constant
 * @param v the value of the constant
 * @param function if not empty, the function to put the definition in
 */
void
ShaderSource::add_const(const std::string &name, std::vector<float> &array,
                        const std::string &function)
{
    std::stringstream ss;

    ss << "const float " << name << "[" << array.size() << "] = {" << std::fixed;
    for(std::vector<float>::const_iterator iter = array.begin();
        iter != array.end();
        iter++)
    {
        ss << *iter;
        if (iter + 1 != array.end())
            ss << ", " << std::endl;
    }

    ss << "};" << std::endl;

    add(ss.str(), function);
}

/** 
 * Adds a vec3 constant definition.
 *
 * @param name the name of the constant
 * @param v the value of the constant
 * @param function if not empty, the function to put the definition in
 */
void
ShaderSource::add_const(const std::string &name, const LibMatrix::vec3 &v,
                        const std::string &function)
{
    std::stringstream ss;

    ss << "const vec3 " << name << " = vec3(" << std::fixed;
    ss << v.x() << ", " << v.y() << ", " << v.z() << ");" << std::endl;

    add(ss.str(), function);
}

/** 
 * Adds a vec4 constant definition.
 *
 * @param name the name of the constant
 * @param v the value of the constant
 * @param function if not empty, the function to put the definition in
 */
void
ShaderSource::add_const(const std::string &name, const LibMatrix::vec4 &v,
                        const std::string &function)
{
    std::stringstream ss;

    ss << "const vec4 " << name << " = vec4(" << std::fixed;
    ss << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ");" << std::endl;

    add(ss.str(), function);
}

/** 
 * Adds a mat3 constant definition.
 *
 * @param name the name of the constant
 * @param v the value of the constant
 * @param function if not empty, the function to put the definition in
 */
void
ShaderSource::add_const(const std::string &name, const LibMatrix::mat3 &m,
                        const std::string &function)
{
    std::stringstream ss;

    ss << "const mat3 " << name << " = mat3(" << std::fixed;
    ss << m[0][0] << ", " << m[1][0] << ", " << m[2][0] << "," << std::endl;
    ss << m[0][1] << ", " << m[1][1] << ", " << m[2][1] << "," << std::endl;
    ss << m[0][2] << ", " << m[1][2] << ", " << m[2][2] << std::endl;
    ss << ");" << std::endl;

    add(ss.str(), function);
}

/** 
 * Adds a float array declaration and initialization.
 *
 * @param name the name of the array
 * @param array the array values
 * @param init_function the function to put the initialization in
 * @param decl_function if not empty, the function to put the declaration in
 */
void
ShaderSource::add_array(const std::string &name, std::vector<float> &array,
                        const std::string &init_function,
                        const std::string &decl_function)
{
    if (init_function.empty() || name.empty())
        return;

    std::stringstream ss;
    ss << "float " << name << "[" << array.size() << "];" << std::endl;

    std::string decl(ss.str());

    ss.clear();
    ss.str("");
    ss << std::fixed;

    for(std::vector<float>::const_iterator iter = array.begin();
        iter != array.end();
        iter++)
    {
        ss << name << "[" << iter - array.begin() << "] = " << *iter << ";" << std::endl; 
    }

    add(ss.str(), init_function);

    add(decl, decl_function);
}

/** 
 * Helper function that emits a precision stastement.
 * 
 * @param ss the stringstream to add the statement to
 * @param val the precision value
 * @param type_str the variable type to apply the precision value to
 * @param is_fragment whether this is targeted for a fragment shader
 */
static void
emit_precision(std::stringstream& ss, ShaderSource::PrecisionValue val,
               const std::string& type_str, bool is_fragment)
{
    static const char *precision_map[] = {
        "lowp", "mediump", "highp", NULL
    };

    if (val == ShaderSource::PrecisionValueHigh) {
        if (is_fragment)
            ss << "#ifdef GL_FRAGMENT_PRECISION_HIGH" << std::endl;

        ss << "precision highp " << type_str << ";" << std::endl;

        if (is_fragment) {
            ss << "#else" << std::endl;
            ss << "precision mediump " << type_str << ";" << std::endl;
            ss << "#endif" << std::endl;
        }
    }
    else if (val >= 0 && val < ShaderSource::PrecisionValueDefault) {
        ss << "precision " << precision_map[val] << " ";
        ss << type_str << ";" << std::endl;
    }

    /* There is no default precision in the fragment shader, so set it to mediump */
    if (val == ShaderSource::PrecisionValueDefault
        && type_str == "float" && is_fragment)
    {
        ss << "precision mediump float;" << std::endl;
    }
}

/** 
 * Gets a string containing the complete shader source.
 *
 * Precision statements are applied at this point.
 * 
 * @return the shader source
 */
std::string
ShaderSource::str()
{
    std::string source(source_.str());

    /* Find out if this is a vertex or fragment shader */
    bool is_fragment = false;

    if (source.find("gl_FragColor") != std::string::npos)
        is_fragment = true;

    /* Decide which precision values to use */
    ShaderSource::Precision precision;

    if (precision_has_been_set_)
        precision = precision_;
    else if (is_fragment)
        precision = default_fragment_precision_;
    else
        precision = default_vertex_precision_;

    /* Create the precision statements */
    std::stringstream ss;
    
    emit_precision(ss, precision.int_precision, "int", is_fragment);
    emit_precision(ss, precision.float_precision, "float", is_fragment);
    emit_precision(ss, precision.sampler2d_precision, "sampler2D", is_fragment);
    emit_precision(ss, precision.samplercube_precision, "samplerCube", is_fragment);

    std::string precision_str(ss.str());
    if (!precision_str.empty()) {
        precision_str.insert(0, "#ifdef GL_ES\n");
        precision_str.insert(precision_str.size(), "#endif\n");
    }

    return precision_str + source;
}

/** 
 * Sets the precision that will be used for this shader.
 *
 * This overrides any default values set with ShaderSource::default_*_precision().
 * 
 * @param precision the precision to set
 */
void
ShaderSource::precision(const ShaderSource::Precision& precision)
{
    precision_ = precision;
    precision_has_been_set_ = true;
}

/** 
 * Gets the precision that will be used for this shader.
 * 
 * @return the precision
 */
const ShaderSource::Precision&
ShaderSource::precision()
{
    return precision_;
}

/** 
 * Sets the default precision that will be used for vertex shaders.
 *
 * This can be overriden per ShaderSource object by using ::precision().
 * 
 * @param precision the default precision to set
 */
void
ShaderSource::default_vertex_precision(const ShaderSource::Precision& precision)
{
    default_vertex_precision_ = precision;
}

/** 
 * Gets the default precision that will be used for vertex shaders.
 * 
 * @return the precision
 */
const ShaderSource::Precision&
ShaderSource::default_vertex_precision()
{
    return default_vertex_precision_;
}

/** 
 * Gets the default precision that will be used for fragment shaders.
 *
 * This can be overriden per ShaderSource object by using ::precision().
 * 
 * @param precision the default precision to set
 */
void
ShaderSource::default_fragment_precision(const ShaderSource::Precision& precision)
{
    default_fragment_precision_ = precision;
}

/** 
 * Gets the default precision that will be used for fragment shaders.
 * 
 * @return the precision
 */
const ShaderSource::Precision&
ShaderSource::default_fragment_precision()
{
    return default_fragment_precision_;
}

/****************************************
 * ShaderSource::Precision constructors *
 ****************************************/

/** 
 * Creates a ShaderSource::Precision with default precision values.
 */
ShaderSource::Precision::Precision() :
    int_precision(ShaderSource::PrecisionValueDefault),
    float_precision(ShaderSource::PrecisionValueDefault),
    sampler2d_precision(ShaderSource::PrecisionValueDefault),
    samplercube_precision(ShaderSource::PrecisionValueDefault)
{
}

/** 
 * Creates a ShaderSource::Precision using the supplied precision values.
 */
ShaderSource::Precision::Precision(ShaderSource::PrecisionValue int_p,
                                   ShaderSource::PrecisionValue float_p,
                                   ShaderSource::PrecisionValue sampler2d_p,
                                   ShaderSource::PrecisionValue samplercube_p) :
    int_precision(int_p), float_precision(float_p),
    sampler2d_precision(sampler2d_p), samplercube_precision(samplercube_p)
{
}

/** 
 * Creates a ShaderSource::Precision from a string representation of
 * precision values.
 *
 * The string format is:
 * "<int>,<float>,<sampler2d>,<samplercube>"
 *
 * Each precision value is one of "high", "medium", "low" or "default".
 * 
 * @param precision_values the string representation of the precision values
 */
ShaderSource::Precision::Precision(const std::string& precision_values) :
    int_precision(ShaderSource::PrecisionValueDefault),
    float_precision(ShaderSource::PrecisionValueDefault),
    sampler2d_precision(ShaderSource::PrecisionValueDefault),
    samplercube_precision(ShaderSource::PrecisionValueDefault)
{
    std::vector<std::string> elems;

    Util::split(precision_values, ',', elems);

    for (size_t i = 0; i < elems.size() && i < 4; i++) {
        const std::string& pstr(elems[i]);
        ShaderSource::PrecisionValue pval;

        if (pstr == "high")
            pval = ShaderSource::PrecisionValueHigh;
        else if (pstr == "medium")
            pval = ShaderSource::PrecisionValueMedium;
        else if (pstr == "low")
            pval = ShaderSource::PrecisionValueLow;
        else
            pval = ShaderSource::PrecisionValueDefault;

        switch(i) {
            case 0: int_precision = pval; break;
            case 1: float_precision = pval; break;
            case 2: sampler2d_precision = pval; break;
            case 3: samplercube_precision = pval; break;
            default: break;
        }
    }
}
