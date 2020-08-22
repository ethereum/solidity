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

#pragma once

#include <random>
#include <regex>
#include <vector>

namespace solidity::test::fuzzer
{
	static std::vector<std::pair<std::string, std::string>> Mutations = {
		{"==", "!="},
		{"!=", "=="},
		{">", "<="},
		{"<", ">="},
		{"<=", ">"},
		{">=", "<"},
		{"\n", "\nbreak;\n"},
		{"break;\n", ""},
		{"\n", "\ncontinue;\n"},
		{"continue;\n", ""}
	};

	struct SRM {
		SRM() {
			for (auto const& item: Mutations)
				addRule(item.first, item.second);
		}

		void addRule(std::string const& _pattern, std::string const& _replaceString)
		{
			RegexpRules.emplace_back(std::pair(std::regex(_pattern), _replaceString));
		}

		size_t mutateInPlace(uint8_t* _data, size_t size, size_t _maxSize, unsigned _ruleIdx);

		std::vector<std::pair<std::regex, std::string>> RegexpRules = {};
	};
}