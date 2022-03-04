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
#include <range/v3/view/split.hpp>
#include <range/v3/to_container.hpp>

using namespace solidity::util;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::optional;
using std::nullopt;
using std::stringstream;
using std::make_pair;

const int verbose = 0;
vector<string> cutStringBySpace(string& input);
optional<vector<Literal>> parseLine(std::string& line);
std::pair<vector<vector<Literal>>, size_t> readCNFFile(const string& fname);
size_t getNumVars(const vector<vector<Literal>>& cls);


vector<string> cutStringBySpace(string& input)
{
    return input | ranges::views::split(' ') | ranges::to<vector<string>>();
}

optional<vector<Literal>> parseLine(std::string& line)
{
	vector<Literal> cl;
	bool endOfClause = false;
	for (auto const& part: line | ranges::views::split(' '))
	{
		if (endOfClause)
		{
			cout << "ERROR: trailing elements after finishing `0` at the end of a clause in CNF file" << endl;
			exit(-1);
		}
		const long lit = std::stol(ranges::to<string>(part));
		const long var = std::abs(lit);
		if (var == 0)
		{
			//end of clause
			endOfClause = true;
			continue;
		}
		assert(var > 0);
		const Literal l {lit > 0, (size_t)var-1};
		cl.push_back(l);
	}

	if (verbose)
	{
		cout << "cl: ";
		for (auto const& l: cl)
			cout << (l.positive ? "" : "-") << (l.variable+1) << " ";
		cout << " end: " << (int)endOfClause << endl;
	}

	if (endOfClause)
		return cl;
	else
		return nullopt;
}

std::pair<vector<vector<Literal>>, size_t> readCNFFile(const string& fname)
{
	long varsByHeader = -1;
	long clsByHeader = -1;
	vector<vector<Literal>> cls;
	std::ifstream infile(fname.c_str());

	std::string line;
	while (std::getline(infile, line))
	{
		if (line.empty())
			continue;
		if (line[0] == 'p') {
			assert(line.substr(0,6) == string("p cnf "));
			line = line.substr(6);
			vector<string> parts = cutStringBySpace(line);
			varsByHeader = std::stol(parts[0]);
			assert(varsByHeader >= 0);
			clsByHeader =  std::stol(parts[1]);
			assert(clsByHeader >= 0);

			continue;
		}
		const auto cl = parseLine(line);
		if (cl)
			cls.push_back(cl.value());
	}
	if (varsByHeader == -1)
	{
		cout << "ERROR: CNF did not have a header" << endl;
		exit(-1);
	}

	assert(clsByHeader >= 0);
	if (cls.size() != (size_t)clsByHeader)
	{
		cout << "ERROR: header said number of clauses will be " << clsByHeader << " but we read " << cls.size() << endl;
		exit(-1);
	}

	return make_pair(cls, (size_t)varsByHeader);
}

size_t getNumVars(const vector<vector<Literal>>& cls)
{
	size_t largestVar = 0;
	for (auto const& cl: cls)
		for (auto const& l: cl)
			largestVar = std::max(largestVar, l.variable+1);

	return largestVar;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << "ERROR: you must give CNF a parameters" << endl;
		exit(-1);
	}

	const string cnfFileName = argv[1];
	auto&& [cls, maxVarsByHeader] = readCNFFile(cnfFileName);
	const size_t numVarsByCls = getNumVars(cls);
	vector<string> variables;

	if (maxVarsByHeader < numVarsByCls)
	{
		cout << "ERROR: header promises less variables than what clauses say" << endl;
		exit(-1);
	}
	assert(maxVarsByHeader >= numVarsByCls);

	for (size_t i = 0; i < maxVarsByHeader; i ++)
		variables.push_back(string("x") + std::to_string(i));

	auto model = CDCL{variables, move(cls)}.solve();

	if (model) {
		const size_t lineBreakAfter = 80;
		stringstream ss;
		ss << "v";
		for(size_t i = 0; i < model->size(); i++) {
			if (ss.str().size() > lineBreakAfter) {
				cout << ss.str() << endl;
				ss.clear();
				ss << "v";
			}
			if (model->at(i) != TriState::unset()) {
				ss << " " << (model->at(i) == TriState{true} ? "" : "-") << i+1;
			}

		}
		cout << ss.str() << " 0" << endl;
		cout << "s SATISFIABLE" << endl;
	} else
		cout << "s UNSATISFIABLE" << endl;

	return 0;
}
