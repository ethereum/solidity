// SPDX-License-Identifier: GPL-3.0
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/AsmData.h>
#include <libsolutil/CommonData.h>
#include <functional>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void ForLoopInitRewriter::operator()(Block& _block)
{
	util::iterateReplacing(
		_block.statements,
		[&](Statement& _stmt) -> std::optional<vector<Statement>>
		{
			if (holds_alternative<ForLoop>(_stmt))
			{
				auto& forLoop = std::get<ForLoop>(_stmt);
				(*this)(forLoop.pre);
				(*this)(forLoop.body);
				(*this)(forLoop.post);
				vector<Statement> rewrite;
				swap(rewrite, forLoop.pre.statements);
				rewrite.emplace_back(move(forLoop));
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
