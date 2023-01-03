/*
 * Copyright © 2023 Collabora Limited
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
 *  Alexandros Frantzis
 */

#ifndef GLMARK2_RESULTS_FILE_
#define GLMARK2_RESULTS_FILE_

#include <string>
#include <memory>

class ResultsFile {
public:
    virtual ~ResultsFile() = default;
    static ResultsFile& get();
    static bool init(const std::string &file);

    virtual std::string type() = 0;
    virtual void begin() = 0;
    virtual void end() = 0;
    virtual void begin_info() = 0;
    virtual void end_info() = 0;
    virtual void begin_benchmark() = 0;
    virtual void end_benchmark() = 0;
    virtual void add_field(const std::string &name, const std::string &value) = 0;

protected:
    ResultsFile() = default;
    static std::unique_ptr<ResultsFile> singleton;
};

#endif
