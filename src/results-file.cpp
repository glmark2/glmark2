/*
 * Copyright © 2022 Collabora Limited
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

#include "results-file.h"

namespace
{

class NullResultsFile : public ResultsFile
{
public:
    std::string type() override { return "null"; }
    void begin() override {}
    void end() override { }
    void begin_info() override {}
    void end_info() override {}
    void begin_benchmark() override {}
    void end_benchmark() override {}
    void add_field(const std::string &name, const std::string &value) override
    {
        static_cast<void>(name);
        static_cast<void>(value);
    }
};

}

std::unique_ptr<ResultsFile> ResultsFile::singleton =
    std::make_unique<NullResultsFile>();

ResultsFile& ResultsFile::get()
{
    return *ResultsFile::singleton;
}
