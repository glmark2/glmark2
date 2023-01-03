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
#include "log.h"

#include <fstream>
#include <sstream>

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

std::string escape_csv_text(const std::string &str)
{
    std::stringstream ss;

    for (auto c : str)
    {
        switch (c)
        {
            case '"': ss << "\"\""; break;
            default: ss << c; break;
        }
    }

    return ss.str();
}

class CSVResultsFile : public ResultsFile
{
public:
    CSVResultsFile(std::ofstream &&fs) : fs{std::move(fs)} {}

    std::string type() override { return "CSV"; }

    void begin() override {}
    void end() override {}

    void begin_info() override
    {
        first_field = true;
    }

    void end_info() override
    {
        fs << std::endl;
    }

    void begin_benchmark() override
    {
        first_field = true;
    }

    void end_benchmark() override
    {
        fs << std::endl;
    }

    void add_field(const std::string &name, const std::string &value) override
    {
        static_cast<void>(name);
        fs << (first_field ? "\"" : ",\"") << escape_csv_text(value) << "\"";
        first_field = false;
    }

private:
    std::ofstream fs;
    bool first_field = true;
};

std::string xml_text_escape(const std::string &str)
{
    std::stringstream ss;

    for (auto c : str)
    {
        switch (c)
        {
            case '<': ss << "&lt;"; break;
            case '>': ss << "&gt;"; break;
            case '&': ss << "&amp;"; break;
            default: ss << c; break;
        }
    }

    return ss.str();
}

class XMLResultsFile : public ResultsFile
{
public:
    XMLResultsFile(std::ofstream &&fs) : fs{std::move(fs)} {}

    std::string type() override { return "XML"; }

    void begin() override
    {
        fs << "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>" << std::endl;
        fs << "<glmark2>" << std::endl;
    }

    void end() override
    {
        fs << "</glmark2>" << std::endl;
    }

    void begin_info() override
    {
        fs << "  <info>" << std::endl;
    }

    void end_info() override
    {
        fs << "  </info>" << std::endl;
    }

    void begin_benchmark() override
    {
        fs << "  <benchmark>" << std::endl;
    }

    void end_benchmark() override
    {
        fs << "  </benchmark>" << std::endl;
    }

    void add_field(const std::string &name, const std::string &value) override
    {
        std::string escaped = xml_text_escape(value);
        fs << "     <" << name << ">" << escaped << "</" << name << ">" << std::endl;
    }

private:
    std::ofstream fs;
};

std::string get_file_extension(const std::string &str)
{
    auto i = str.rfind('.');
    if (i != std::string::npos)
        return str.substr(i);
    else
        return {};
}

}

std::unique_ptr<ResultsFile> ResultsFile::singleton =
    std::make_unique<NullResultsFile>();

bool ResultsFile::init(const std::string &file)
{
    std::ofstream fs{file};
    std::unique_ptr<ResultsFile> results_file;
    std::string ext = get_file_extension(file);

    if (file.empty())
        return true;

    if (!fs)
    {
        Log::error("Failed to open results file %s\n", file.c_str());
        return false;
    }

    if (ext.empty())
    {
        Log::error("Failed to determine results file type for %s\n",
                   file.c_str());
        return false;
    }

    if (ext == ".csv")
    {
        ResultsFile::singleton = std::make_unique<CSVResultsFile>(std::move(fs));
    }
    else if (ext == ".xml")
    {
        ResultsFile::singleton = std::make_unique<XMLResultsFile>(std::move(fs));
    }
    else
    {
        Log::error("Results file type %s is not supported\n", file.c_str());
        return false;
    }

    Log::debug("Writing results to %s file %s\n",
               ResultsFile::get().type().c_str(), file.c_str());

    return true;
}

ResultsFile& ResultsFile::get()
{
    return *ResultsFile::singleton;
}
