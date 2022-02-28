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

using namespace solidity::util;
using std::vector;
using std::string;
using std::cout;
using std::endl;

Literal lit(string const& _name);

vector<string> m_variables;

Literal lit(string const& _name)
{
	m_variables.emplace_back(_name);
	return Literal{true, m_variables.size() - 1};
}

int main()
{
	std::ofstream proofFile;
	proofFile.open("proof.frat", std::ios::out);

	auto x1 = lit("x1");
	auto x2 = lit("x2");
	vector<Literal> c1{x1, ~x2};
	vector<Literal> c2{~x1, x2};
	vector<Literal> c3{x1, x2};
	vector<Literal> c4{~x1, ~x2};
	const auto cls = {c1, c2, c3, c4};
	auto model = CDCL{m_variables, move(cls), &proofFile}.solve();

	if (model) {
		cout << "SATISFIABLE" << endl;
	} else {
		cout << "UNSATISFIABLE" << endl;
	}
	proofFile.close();
	return 0;
}
