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

#include <test/tools/ossfuzz/antlr4-runtime/SolidityMutator.h>

#include <libsolutil/Whiskers.h>

#include <boost/algorithm/string/erase.hpp>

using namespace std;
using namespace solidity::util;

antlrcpp::Any SolidityMutator::visitPragmaDirective(SolidityParser::PragmaDirectiveContext* _ctx)
{
	auto PickLiteral = [](unsigned const len) -> string
	{
		// TODO: Add to this list of valid pragmas
		static const vector<string> alphanum = {
			"solidity >=0.0.0",
			"solidity ^0.4.24",
			"experimental SMTChecker",
			"experimental ABIEncoderV2",
		};

		return alphanum[len % alphanum.size()];
	};

	string pragma{};
	for (auto &t: _ctx->PragmaToken())
	{
		string literal = t->toString();
		boost::algorithm::erase_all(literal, " ");
		if (random() % 5 == 0)
			pragma += " " + PickLiteral(random());
		else
			pragma += " " + literal;
	}

	Out << Whiskers(R"(pragma <string>;<nl>)")
		("string", pragma)
		("nl", "\n")
		.render();

	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitImportDirective(SolidityParser::ImportDirectiveContext* _ctx)
{
	if (_ctx->path())
		Out << Whiskers(R"(import <path> as <id>;<nl>)")
			("path", "\"A.sol\"")
			("id", "A")
			("nl", "\n")
			.render();
	else if (_ctx->Mul())
		Out << Whiskers(R"(import * as <id> from <path>;<nl>)")
			("id", "A")
			("path", "\"A.sol\"")
			("nl", "\n")
			.render();
	else if (_ctx->symbolAliases())
		Out << Whiskers(R"(import {<symbolAliases>} from <path>;<nl>)")
			("symbolAliases", "C as B, A")
			("path", "\"A.sol\"")
			("nl", "\n")
			.render();
	else
		Out << Whiskers(R"(import "A.sol";<nl>)")
				("nl", "\n")
			   .render();

	return antlrcpp::Any();
}