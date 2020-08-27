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

#include <test/tools/ossfuzz/SolidityMutator.h>

#include <algorithm>

using namespace std;
using namespace solidity::util;
using namespace solidity::test::fuzzer;

antlrcpp::Any SolidityMutator::visitSourceUnit(SolidityParser::SourceUnitContext* _ctx)
{
	if (_ctx->pragmaDirective().empty())
		m_out << Whiskers(s_pragmaDirective)("directive", "experimental SMTChecker").render();
	else
		visitChildren(_ctx);
	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitPragmaDirective(SolidityParser::PragmaDirectiveContext*)
{
	auto PickLiteral = [&]() -> string
	{
		static const vector<string> alphanum = {
			"0",
			"experimental",
			"solidity < 142857",
			"solidity >=0.0.0",
			"solidity >=0.0.0 <0.8.0",
			"experimental __test",
			"experimental SMTChecker",
			"experimental ABIEncoderV2",
			"experimental \"xyz\""
		};

		return alphanum[randomOneToN(alphanum.size()) - 1];
	};

	string pragma = PickLiteral();

	m_out << Whiskers(s_pragmaDirective)("directive", pragma).render();
	return antlrcpp::Any();
}