/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Alexander <mail@akru.me>
 * @date 2016
 * Solidity commandline doc generator.
 */

#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>

#include "format.h" 

using namespace std;
using namespace dev::solidity;
using namespace boost::filesystem;
using namespace boost::program_options;

void find_packages(
        vector<path> &files,
        vector<string> const& dirs,
        string const& ext
) {
    directory_iterator end_itr;
    for (auto dir: dirs) {

        for (directory_iterator package_dir(dir);
             package_dir != end_itr;
             ++package_dir) {

            if (is_directory(package_dir->status())) {

                for (directory_iterator source_file(package_dir->path());
                     source_file != end_itr;
                     ++source_file) {

                    if (!is_directory(source_file->status())
                            && source_file->path().extension() == ext)
                        files.push_back(source_file->path());
                }
            }
        }
    }
}

void read_packages(
        map<string, string> &sources,
        vector<path> const& source_files
) {
    for (auto source: source_files) {
        stringstream full_name;
        full_name << source.parent_path().filename().string()
                  << "/"
                  << source.filename().string();

        ifstream source_stream(source.string());
        string content(istreambuf_iterator<char>(source_stream), {});
        sources[full_name.str()] = content;
    }
}

int main(int argc, char** argv)
{
    options_description options("Allowed options");
    options.add_options()
        ("help", "print this message and exit")
        ("include-path,I", value<vector<string>>()->composing(), "add the directory to the list of directories to be searched for Solidity files")
        ("contract,C", value<string>(), "select contract by name")
    ;
    variables_map vm;
    store(parse_command_line(argc, argv, options), vm);
    notify(vm);

    if (vm.count("help") || !vm.count("include-path"))
    {
        cout << options << endl;
        return 1;
    }

    auto includes = vm["include-path"].as<vector<string>>();
    
    vector<path> package_files;
    find_packages(package_files, includes, ".sol");

    map<string, string> sources;
    read_packages(sources, package_files);

    CompilerStack solc;
    solc.addSources(sources);

    if(!solc.parse())
    {
        cerr << "Errors:" << endl;
        auto scannerFromSourceName = [&](string const& _sourceName) -> Scanner const& { return solc.scanner(_sourceName); };
        for (auto err: solc.errors())
            SourceReferenceFormatter::printExceptionInformation(cerr, *err,
                    (err->type() == Error::Type::Warning) ? "Warning" : "Error",
                    scannerFromSourceName);
        return 1;
    }

    if (vm.count("contract"))
    {
        auto name = vm["contract"].as<string>();
        cout << formatMarkdown(solc.contractDefinition(name)) << endl;
    }
    else
    {
        for (auto name: solc.contractNames())
            cout << navigationMarkdown(solc.contractDefinition(name));
        cout << endl;

        for (auto name: solc.contractNames())
            cout << formatMarkdown(solc.contractDefinition(name)) << endl;
    }

    return 0;
}
