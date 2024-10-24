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

#include <libyul/interpreter/PureInterpreter.h>

#include <libyul/AST.h>

#include <libsolutil/Visitor.h>

#include <liblangutil/Exceptions.h>

#include <string_view>

using namespace solidity::yul::interpreter;

void PureInterpreterState::dumpTraces(std::ostream& _out) const
{
	static std::string_view const INDENT = "  ";
	static std::string_view const BAR = "│ ";

	_out << "Call trace:\n";
	std::vector<FunctionCallTrace const*> stackTrace;

	auto print_values = [&](std::vector<u256> const& values)
	{
		bool isFirst = true;
		for (u256 value: values)
		{
			if (!isFirst)
				_out << ", ";
			isFirst = false;
			_out << value;
		}
	};

	for (auto const& trace: traces) {
		_out << INDENT;
		for (size_t i = 0, callLevel = stackTrace.size(); i < callLevel; ++i)
			_out << BAR;

		std::visit(util::GenericVisitor{
			[&](FunctionCallTrace const& callTrace) {
				_out << "[CALL] " << callTrace.definition.name.str() << "(";
				print_values(callTrace.params);
				_out << ")";
				stackTrace.push_back(&callTrace);
			},
			[&](FunctionReturnTrace const& returnTrace) {
				solAssert(!stackTrace.empty());
				bool areIdentical = &stackTrace.back()->definition == &returnTrace.definition;
				solAssert(areIdentical);

				_out << "[RETURN]";
				if (!returnTrace.returnedValues.empty())
					_out << " ";
				print_values(returnTrace.returnedValues);
				stackTrace.pop_back();
			}
		}, trace);

		_out << std::endl;
	}
}
