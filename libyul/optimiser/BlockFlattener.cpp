// SPDX-License-Identifier: GPL-3.0
#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/AsmData.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/CommonData.h>
#include <functional>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

void BlockFlattener::operator()(Block& _block)
{
	ASTModifier::operator()(_block);

	iterateReplacing(
		_block.statements,
		[](Statement& _s) -> std::optional<vector<Statement>>
		{
			if (holds_alternative<Block>(_s))
				return std::move(std::get<Block>(_s).statements);
			else
				return {};
		}
	);
}
