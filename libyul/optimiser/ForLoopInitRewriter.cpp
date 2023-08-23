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
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

#include <functional>

using namespace solidity;
using namespace solidity::yul;

void ForLoopInitRewriter::operator()(Block& _block)
{
	util::iterateReplacing(
		_block.statements,
		[&](Statement& _stmt) -> std::optional<std::vector<Statement>>
		{
			if (std::holds_alternative<ForLoop>(_stmt))
			{
				auto& forLoop = std::get<ForLoop>(_stmt);
				(*this)(forLoop.pre);
				(*this)(forLoop.body);
				(*this)(forLoop.post);
				std::vector<Statement> rewrite;
				swap(rewrite, forLoop.pre.statements);
				rewrite.emplace_back(std::move(forLoop));
				return { std::move(rewrite) };
			}
			else
			{
				visit(_stmt);
				return {};
			}
		}
	);
}
