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

#include <string>
#include <sstream>
#include <vector>
#include "vec.h"
#include "mat.h"

/** 
 * Helper class for loading and manipulating shader sources.
 */
class ShaderSource
{
public:
    enum ShaderType {
        ShaderTypeVertex,
        ShaderTypeFragment,
        ShaderTypeUnknown
    };

    ShaderSource(ShaderType type = ShaderTypeUnknown) :
        precision_has_been_set_(false), type_(type) {}
    ShaderSource(const std::string &filename, ShaderType type = ShaderTypeUnknown) :
        precision_has_been_set_(false), type_(type) { append_file(filename); }

    void append(const std::string &str);
    void append_file(const std::string &filename);

    void replace(const std::string &remove, const std::string &insert);
    void replace_with_file(const std::string &remove, const std::string &filename);

    void add(const std::string &str, const std::string &function = "");

    void add_const(const std::string &name, float f,
                   const std::string &function = "");
    void add_const(const std::string &name, std::vector<float> &f,
                   const std::string &function = "");
    void add_const(const std::string &name, const LibMatrix::vec3 &v,
                   const std::string &function = "");
    void add_const(const std::string &name, const LibMatrix::vec4 &v,
                   const std::string &function = "");
    void add_const(const std::string &name, const LibMatrix::mat3 &m,
                   const std::string &function = "");

    void add_array(const std::string &name, std::vector<float> &array,
                   const std::string &init_function,
                   const std::string &decl_function = "");

    ShaderType type();
    std::string str();

    enum PrecisionValue {
        PrecisionValueLow,
        PrecisionValueMedium,
        PrecisionValueHigh,
        PrecisionValueDefault,
    };

    struct Precision {
        Precision();
        Precision(PrecisionValue int_p, PrecisionValue float_p,
                  PrecisionValue sampler2d_p, PrecisionValue samplercube_p);
        Precision(const std::string& list);

        PrecisionValue int_precision;
        PrecisionValue float_precision;
        PrecisionValue sampler2d_precision;
        PrecisionValue samplercube_precision;
    };

    void precision(const Precision& precision);
    const Precision& precision();

    static void default_precision(const Precision& precision,
                                  ShaderType type = ShaderTypeUnknown);
    static const Precision& default_precision(ShaderType type);

private:
    void add_global(const std::string &str);
    void add_local(const std::string &str, const std::string &function);
    bool load_file(const std::string& filename, std::string& str);
    void emit_precision(std::stringstream& ss, ShaderSource::PrecisionValue val,
                        const std::string& type_str);

    std::stringstream source_;
    Precision precision_;
    bool precision_has_been_set_;
    ShaderType type_;

    static std::vector<Precision> default_precision_;
};
