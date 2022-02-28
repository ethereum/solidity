/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#include <libsolutil/CDCL.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace solidity::util;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::optional;
using std::nullopt;
using std::make_pair;

const int verbose = 0;
vector<string> cut_string_by_space(string& input);
optional<vector<Literal>> parse_line(std::string& line);
std::pair<vector<vector<Literal>>, size_t> read_cnf_file(const string& fname);
size_t get_num_vars(const vector<vector<Literal>>& cls);


vector<string> cut_string_by_space(string& input)
{
    vector<string> parts;

    size_t pos = 0;
    while ((pos = input.find(" ")) != string::npos) {
        parts.push_back(input.substr(0, pos));
        input.erase(0, pos + 1);
    }
    parts.push_back(input);
    return parts;
}

optional<vector<Literal>> parse_line(std::string& line)
{
	vector<Literal> cl;
	bool end_of_clause = false;
	auto parts = cut_string_by_space(line);
	for(const auto& part: parts) {
		assert(!end_of_clause);

		const long lit = std::stol(part);
		const long var = std::abs(lit);
		if (var == 0) {
			end_of_clause = true;
			//end of clause
			continue;
		}
		assert(var > 0);
		const Literal l {lit > 0, (size_t)var-1};
		cl.push_back(l);
	}

	if (verbose) {
		cout << "Cl: ";
		for(const auto& l: cl) {
			cout << (l.positive ? "" : "-") << (l.variable+1) << " ";
		}
		cout << " end: " << (int)end_of_clause << endl;
	}

	if (end_of_clause) {
		return cl;
	} else {
		return nullopt;
	}
}

std::pair<vector<vector<Literal>>, size_t> read_cnf_file(const string& fname)
{
	long varsByHeader = -1;
	long clsByHeader = -1;
	vector<vector<Literal>> cls;
	std::ifstream infile(fname.c_str());

	std::string line;
	while (std::getline(infile, line))
	{
		if (line.empty()) {
			continue;
		}
		if (line[0] == 'p') {
			assert(line.substr(0,6) == string("p cnf "));
			line = line.substr(6);
			vector<string> parts = cut_string_by_space(line);
			varsByHeader = std::stol(parts[0]);
			assert(varsByHeader >= 0);
			clsByHeader =  std::stol(parts[1]);
			assert(clsByHeader >= 0);

			continue;
		}
		const auto cl = parse_line(line);
		if (cl) {
			cls.push_back(cl.value());
		}
	}
	if (varsByHeader == -1) {
		cout << "ERROR: CNF did not have a header" << endl;
		exit(-1);
	}

	assert(clsByHeader >= 0);
	if (cls.size() != (size_t)clsByHeader) {
		cout << "ERROR: header said number of clauses will be " << clsByHeader << " but we read " << cls.size() << endl;
		exit(-1);
	}

	return make_pair(cls, (size_t)varsByHeader);
}

size_t get_num_vars(const vector<vector<Literal>>& cls)
{
	size_t largestVar = 0;
	for(const auto& cl: cls) {
		for(const auto& l: cl) {
			largestVar = std::max(largestVar, l.variable+1);
		}
	}
	return largestVar;
}

int main(int argc, char** argv)
{
	if (argc != 3) {
		cout << "ERROR: you must give CNF and proof files as parameters" << endl;
		exit(-1);
	}

	const string cnfFileName = argv[1];
	const string proofFileName = argv[2];
	auto&& [cls, maxVarsByHeader] = read_cnf_file(cnfFileName);
	std::ofstream proofFile;
	proofFile.open(proofFileName.c_str(), std::ios::out);
	const size_t numVarsByCls = get_num_vars(cls);
	vector<string> m_variables;

	if (maxVarsByHeader < numVarsByCls) {
		cout << "ERROR: header promises less variables than what clauses say" << endl;
		exit(-1);
	}
	assert(maxVarsByHeader >= numVarsByCls);

	for(size_t i = 0; i < maxVarsByHeader; i ++) {
		m_variables.push_back(string("x") + std::to_string(i));
	}

	auto model = CDCL{m_variables, move(cls), &proofFile}.solve();

	if (model) {
		cout << "s SATISFIABLE" << endl;
	} else {
		cout << "s UNSATISFIABLE" << endl;
	}
	proofFile.close();
	return 0;
}
