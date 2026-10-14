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

#include <libyul/backends/evm/ssa/io/Keywords.h>

#include <libyul/Exceptions.h>

#include <array>
#include <utility>

using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::io;

namespace
{

constexpr std::array<std::pair<Keyword, std::string_view>, 22> keywordStringRepresentations{{
	{Keyword::MemoryGuard, "memoryguard"},
	{Keyword::Func, "func"},
	{Keyword::Args, "args"},
	{Keyword::NoContinue, "nocontinue"},
	{Keyword::Preds, "preds"},
	{Keyword::Builtin, "builtin"},
	{Keyword::Call, "call"},
	{Keyword::Upsilon, "upsilon"},
	{Keyword::Const, "const"},
	{Keyword::Phi, "phi"},
	{Keyword::Arg, "arg"},
	{Keyword::Proj, "proj"},
	{Keyword::Identity, "identity"},
	{Keyword::Nop, "nop"},
	{Keyword::Unreachable, "unreachable"},
	{Keyword::Tombstone, "tombstone"},
	{Keyword::True, "true"},
	{Keyword::Jump, "jump"},
	{Keyword::Branch, "branch"},
	{Keyword::Return, "return"},
	{Keyword::MainExit, "main_exit"},
	{Keyword::Terminated, "terminated"},
}};

static_assert(
	[] {
		for (std::size_t i = 0; i < keywordStringRepresentations.size(); ++i)
			if (static_cast<std::size_t>(keywordStringRepresentations[i].first) != i)
				return false;
		return keywordStringRepresentations.back().first == Keyword::Terminated;
	}(),
	"keywordStringRepresentations must list every Keyword in declaration order."
);

}

std::string_view io::keywordToString(Keyword const _keyword)
{
	auto const index = static_cast<std::size_t>(_keyword);
	yulAssert(index < keywordStringRepresentations.size());
	return keywordStringRepresentations[index].second;
}

std::optional<Keyword> io::keywordFromString(std::string_view const _spelling)
{
	for (auto const& [keyword, spelling]: keywordStringRepresentations)
		if (spelling == _spelling)
			return keyword;
	return std::nullopt;
}
