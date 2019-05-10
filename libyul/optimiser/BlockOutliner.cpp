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
/**
 * Optimiser component that outlines blocks that occur multiple times.
 */

#include <libyul/optimiser/BlockOutliner.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/AsmData.h>
#include <libdevcore/CommonData.h>
#include <libyul/AsmPrinter.h>
#include <libdevcore/AnsiColorized.h>
#include <libdevcore/StringUtils.h>

using namespace std;
using namespace dev;
using namespace yul;

bool BlockOutliner::shallOutline(BlockClass const& _blockClass)
{
	if (_blockClass.hasFreeBreakOrContinue)
		return false;
	if (_blockClass.members.size() < 2)
		return false;
	// outline everything for now for testing
	// TODO: find good heuristics
	return true;
	/*
	auto codeSize = CodeSize::codeSize(*_blockClass.members.front().block);
	auto const& representative = _blockClass.members.front();
	if (representative.externalAssignments.size() > 5)
		return false;
	if (representative.externalReads.size() > 5)
		return false;
	if (representative.externalAssignments.size() > 4)
		return codeSize >= 15;
	if (representative.externalReads.size() > 4)
		return codeSize >= 15;
	return codeSize >= 7;
	 */
}

void BlockOutliner::run(Block& _ast, NameDispenser& _nameDispenser)
{
	std::vector<BlockClass> blockClasses = BlockClassFinder::run(_ast);
	std::map<Block const*, Statement> blockToFunctionCall;
	std::vector<pair<BlockClass const*, YulString>> outlinedBlockClasses;

	for (auto const& blockClass: blockClasses)
	{
		if (!shallOutline(blockClass))
			continue;

		YulString nameHint = blockClass.nameHint;
		if (nameHint.empty())
			nameHint = YulString(
				"outlined$" +
				to_string(blockClass.members.front().block->location.start) +
				"$"
			);
		outlinedBlockClasses.emplace_back(&blockClass, _nameDispenser.newName(nameHint));

		// generate a function call for each block in the class
		for (auto const& block: blockClass.members)
		{
			auto loc = block.block->location;
			vector<Expression> arguments;
			vector<Identifier> identifiers;
			for (auto const& name: block.externalReferences)
			{
				if (block.externalAssignments.count(name))
					identifiers.emplace_back(Identifier{loc, name});
				if (block.externalReads.count(name))
					arguments.emplace_back(Identifier{loc, name});
			}
			FunctionCall call{
				loc,
				Identifier{loc, outlinedBlockClasses.back().second},
				std::move(arguments)
			};
			if (identifiers.empty())
				blockToFunctionCall[block.block] = ExpressionStatement{loc, move(call)};
			else
				blockToFunctionCall[block.block] = Assignment{
					loc, move(identifiers), make_unique<Expression>(move(call))
				};
		}
	}

	if (!outlinedBlockClasses.empty())
	{
		BlockOutliner outliner{std::move(blockToFunctionCall), _nameDispenser};
		Block astCopy = boost::get<Block>(outliner(_ast));
		for (auto const& outline: outlinedBlockClasses)
			astCopy.statements.emplace_back(
				outliner.blockClassToFunction(*outline.first, outline.second)
			);
		_ast = std::move(astCopy);
	}
}

Block BlockOutliner::translate(Block const& _block)
{
	auto it = m_blockOutlines.find(&_block);
	if (it != m_blockOutlines.end())
		return Block {
			_block.location,
			make_vector<Statement>(std::move(it->second))
		};
	return ASTCopier::translate(_block);
}

FunctionDefinition BlockOutliner::blockClassToFunction(
	BlockClass const& _blockClass,
	YulString _functionName
)
{
	yulAssert(!_blockClass.members.empty(), "");
	Block const& _block = *_blockClass.members.front().block;
	Block body{_block.location, translateVector(_block.statements)};

	TypedNameList parameters;
	TypedNameList returnVariables;
	for (auto const& name: _blockClass.members.front().externalReferences)
	{
		bool isRead = _blockClass.members.front().externalReads.count(name);
		bool isWritten = _blockClass.members.front().externalAssignments.count(name);
		if (isRead)
			parameters.emplace_back(TypedName{_block.location, name, {}});
		if (isWritten)
		{
			if (!isRead)
				returnVariables.emplace_back(TypedName{
					_block.location,
					name,
					{}
				});
			else
			{
				returnVariables.emplace_back(TypedName{
					_block.location,
					m_nameDispenser.newName(name),
					{}
				});
				body.statements.emplace_back(Assignment{
					_block.location,
					{Identifier{_block.location, returnVariables.back().name}},
					make_unique<Expression>(Identifier{_block.location, name})
				});
			}
		}
	}
	return FunctionDefinition{
		_block.location,
		_functionName,
		move(parameters),
		move(returnVariables),
		move(body)
	};
}
