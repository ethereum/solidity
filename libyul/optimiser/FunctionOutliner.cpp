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

#include <libyul/optimiser/FunctionOutliner.h>
#include <libyul/AST.h>
#include <libsolutil/CommonData.h>

#include <libyul/optimiser/Metrics.h>

#include <libyul/AsmPrinter.h>

#include <libyul/optimiser/FullInliner.h>

#include <libsolutil/CommonData.h>

#include <range/v3/view/reverse.hpp>

#include <iostream>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

/**
 * Copies statements and replaces all variables by new names, not only
 * for declarations.
 */
class StatementSequenceCopier: public ASTCopier
{
public:
	explicit StatementSequenceCopier(NameDispenser& _dispenser): m_nameDispenser(_dispenser) {}

	using ASTCopier::operator();
	Statement operator()(VariableDeclaration const& _varDecl) override
	{
		for (TypedName const& var: _varDecl.variables)
		{
			m_declaredHere.insert(var.name);
			markWritten(var.name);
		}
		return ASTCopier::operator()(_varDecl);
	}

	Statement operator()(Assignment const& _assignment) override
	{
		for (Identifier const& var: _assignment.variableNames)
			markWritten(var.name);
		return ASTCopier::operator()(_assignment);
	}

	Statement operator()(FunctionDefinition const&) override
	{
		solAssert(false);
	}

	Expression operator()(Identifier const& _identifier) override
	{
		markRead(_identifier.name);
		return ASTCopier::operator()(_identifier);
	}

	YulString translateIdentifier(YulString _name) override
	{
		if (m_translations.count(_name))
			return m_translations.at(_name);
		return _name;
	}

	void addTranslation(YulString _name)
	{
		if (!m_translations.count(_name))
			m_translations[_name] = m_nameDispenser.newName(_name);
	}
	void markRead(YulString _name)
	{
		addTranslation(_name);
		if (!m_declaredHere.count(_name) && !util::contains(m_parameters, _name))
			m_parameters.emplace_back(_name);
	}
	void markWritten(YulString _name)
	{
		addTranslation(_name);
		if (!util::contains(m_returnParameters, _name))
			m_returnParameters.emplace_back(_name);
	}

	NameDispenser& m_nameDispenser;
	map<YulString, YulString> m_translations = {};
	set<YulString> m_declaredHere = {};
	vector<YulString> m_parameters = {};
	vector<YulString> m_returnParameters = {};
};

class StatementSequenceHasher: public ASTModifier
{
public:
	void operator()(Block& _block) override
	{
		// TODO the hasher should skip function definitions
		ASTModifier::operator()(_block);
		for (size_t i = 0; i < _block.statements.size(); ++i)
		{
			std::map<Block const*, uint64_t> hashes;
			BlockHasher blockHasher(hashes);
			for (size_t j = i; j < _block.statements.size(); ++j)
			{
				blockHasher.visit(_block.statements[j]);
				m_hashes[{&_block, i, j - i + 1}] = blockHasher.currentHash();
			}
		}
	}

//private:
	map<tuple<Block*, size_t, size_t>, uint64_t> m_hashes;
};

namespace
{

size_t sequenceSize(Block const& _block, size_t _start, size_t _length)
{
	size_t size = 0;
	for (size_t i = 0; i < _length; ++i)
		size += CodeSize::codeSize(_block.statements[_start + i]);
	return size;
}

}

void FunctionOutliner::run(OptimiserStepContext& _context, Block& _ast)
{
	StatementSequenceHasher hasher;
	hasher(_ast);

	// TODO make this deterministic

	map<tuple<size_t, uint64_t>, vector<tuple<Block*, size_t, size_t>>> byHash;

	// TODO can we do it in a way such that we can detect which blocks (and their sub-blocks)
	// are invalidated?

	for (auto&& [key, hash]: hasher.m_hashes)
	{
		auto&& [block, start, length] = key;
		byHash[{sequenceSize(*block, start, length), hash}].emplace_back(block, start, length);
	}

	for (auto&& [key, value]: byHash | ranges::views::reverse)
	{
		if (value.size() <= 4)
			continue;

		// TODO check that they are actually syntactically equivalent.

		// TODO check that there is no `break`, `continue` or `leave`


		cout << "-------------------------------------------" << endl;
		cout << value.size() << " items" << endl;
		cout << "code size: " << get<0>(key) << endl;
		for (auto&& [block, start, length]: value)
		{
			cout << " candidate: ------------ " << endl;
			AsmPrinter p;
			for (size_t i = 0; i < length; i++)
				cout << "   " << std::visit(p, block->statements[start + i]) << endl;
			cout << "----" << endl;
		}

		StatementSequenceCopier copier(_context.dispenser);
		auto&& [block, start, length] = value.front();
		FunctionDefinition outlined;
		for (size_t i = 0; i < length; i++)
			outlined.body.statements.emplace_back(
				std::visit(copier, block->statements[start + i])
			);
		for (auto&& param: copier.m_parameters)
			outlined.parameters.emplace_back(TypedName{DebugData::create(), copier.m_translations.at(param), {}});

		for (auto&& param: copier.m_returnParameters)
		{
			YulString newName = _context.dispenser.newName(param);
			outlined.returnVariables.emplace_back(TypedName{DebugData::create(), newName, {}});
			outlined.body.statements.emplace_back(
				Assignment{
					DebugData::create(),
					{Identifier{DebugData::create(), newName}},
					make_unique<Expression>(
						Identifier{DebugData::create(), copier.m_translations.at(param)}
					)
				}
			);

			outlined.returnVariables.emplace_back(TypedName{DebugData::create(), copier.m_translations.at(param), {}});
		}
		outlined.debugData = DebugData::create();
		outlined.name = _context.dispenser.newName(YulString{"outlined"});

		// TODO handle case where replacements happen in the same block.

		vector<Expression> arguments;
		for (YulString argName: copier.m_parameters)
			arguments.emplace_back(Identifier{DebugData::create(), argName});
		FunctionCall functionCall{
			DebugData::create(),
			Identifier{DebugData::create(), outlined.name},
			move(arguments)
		};

		vector<Statement> newStatements;
		if (outlined.returnVariables.empty())
			newStatements.emplace_back(ExpressionStatement{
				DebugData::create(),
				move(functionCall)
			});
		else
		{
			vector<Identifier> assigned;
			for (YulString retName: copier.m_returnParameters)
				assigned.emplace_back(Identifier{DebugData::create(), retName});

			// TODO not deterministic
			for (YulString createdVar: copier.m_declaredHere)
				newStatements.emplace_back(VariableDeclaration{
					DebugData::create(),
					{TypedName{DebugData::create(), createdVar, {}}},
					{}
				});
			newStatements.emplace_back(Assignment{
				DebugData::create(),
				move(assigned),
				make_unique<Expression>(move(functionCall))
			});
		}
		cout << "===========before==============\n" << AsmPrinter{}(_ast) << endl;

		block->statements.erase(
			block->statements.begin() + static_cast<unsigned>(start),
			block->statements.begin() + static_cast<unsigned>(start + length)
		);
		block->statements.insert(
			block->statements.begin() + static_cast<unsigned>(start),
			make_move_iterator(newStatements.begin()),
			make_move_iterator(newStatements.end())
		);

		// TODO also replace the other code.

		_ast.statements.emplace_back(move(outlined));
		cout << "===========after==============\n" << AsmPrinter{}(_ast) << endl;
		cout << "============end============\n";
		return;
	}

}

