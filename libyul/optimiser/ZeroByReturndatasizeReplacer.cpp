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
/**
 * Optimisation stage that replaces the literal 0 by returndatasize() in case there could not have been a call yet.
 */
#include <libyul/optimiser/ZeroByReturndatasizeReplacer.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/SideEffects.h>
#include <libyul/Utilities.h>
#include <libyul/AST.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/reverse.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

namespace
{
bool invalidatesReturndata(Dialect const& _dialect, map<YulString, SideEffects> const& _functionSideEffects, FunctionCall const& _funCall) {
	if (BuiltinFunction const* builtin = _dialect.builtin(_funCall.functionName.name))
		if (builtin->sideEffects.returndata == SideEffects::Write)
			return true;
	if (auto const* sideEffects = util::valueOrNullptr(_functionSideEffects, _funCall.functionName.name))
		if (sideEffects->returndata == SideEffects::Write)
			return true;
	return false;
}

class CallTracer: public ASTModifier
{
public:
	CallTracer(
		Dialect const& _dialect,
		map<YulString, SideEffects> const& _functionSideEffects,
		map<YulString, set<YulString>> const& _functionCalls
	): m_dialect(_dialect), m_functionSideEffects(move(_functionSideEffects)), m_functionCalls(move(_functionCalls))
	{}

	using ASTModifier::operator();
	void operator()(FunctionCall& _funCall) override
	{
		ASTModifier::operator()(_funCall);
		if (!m_hasZeroReturndata)
			util::BreadthFirstSearch<YulString>{{_funCall.functionName.name}, m_badFunctions}.run([&](YulString _fn, auto&& _addChild) {
				m_badFunctions.insert(_fn);
				if (set<YulString> const* callees = util::valueOrNullptr(m_functionCalls, _fn))
					for (YulString f: *callees)
						_addChild(f);
			});

		if (invalidatesReturndata(m_dialect, m_functionSideEffects, _funCall))
			m_hasZeroReturndata = false;
	}
	void operator()(ForLoop& _loop) override
	{
		bool hadZeroReturndata = m_hasZeroReturndata;
		ASTModifier::operator()(_loop);
		if (hadZeroReturndata && !m_hasZeroReturndata)
		{
			m_badLoops.insert(&_loop);
			ASTModifier::operator()(_loop);
		}
	}
	void operator()(FunctionDefinition& _funDef) override
	{
		bool hadZeroReturndata = m_hasZeroReturndata;
		m_hasZeroReturndata = true;
		ASTModifier::operator()(_funDef);
		m_hasZeroReturndata = hadZeroReturndata;
	}

	set<YulString> const& badFunctions() const { return m_badFunctions; }
	set<ForLoop const*> const& badLoops() const { return m_badLoops; }
private:
	set<YulString> m_badFunctions;
	set<ForLoop const*> m_badLoops;
	Dialect const& m_dialect;
	map<YulString, SideEffects> const& m_functionSideEffects;
	map<YulString, set<YulString>> const& m_functionCalls;
	bool m_hasZeroReturndata = true;
};
}

void ZeroByReturndatasizeReplacer::operator()(FunctionCall& _funCall)
{
	BuiltinFunction const* builtin = m_dialect.builtin(_funCall.functionName.name);
	for (auto&& [index, arg]: ranges::views::enumerate(_funCall.arguments) | ranges::views::reverse)
		if (!builtin || !builtin->literalArgument(index))
			visit(arg);
	if (invalidatesReturndata(m_dialect, m_functionSideEffects, _funCall))
		m_hasZeroReturndata = false;
}

void ZeroByReturndatasizeReplacer::operator()(FunctionDefinition& _funDef)
{
	if (!m_badFunctions.count(_funDef.name))
	{
		bool hadZeroReturndata = m_hasZeroReturndata;
		m_hasZeroReturndata = true;
		ASTModifier::operator()(_funDef);
		m_hasZeroReturndata = hadZeroReturndata;
	}
}

void ZeroByReturndatasizeReplacer::operator()(ForLoop& _loop)
{
	if (m_badLoops.count(&_loop))
		m_hasZeroReturndata = false;
	else
		ASTModifier::operator()(_loop);
}


void ZeroByReturndatasizeReplacer::visit(Expression& _e)
{
	if (
		Literal* literal = std::get_if<Literal>(&_e);
		m_hasZeroReturndata && literal && valueOfLiteral(*literal) == 0
	)
		_e = FunctionCall{
			literal->location,
			Identifier{literal->location, "returndatasize"_yulstring},
			{}
		};
	else
		ASTModifier::visit(_e);
}

void ZeroByReturndatasizeReplacer::run(OptimiserStepContext& _context, Block& _ast)
{
	if (
		EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
		!dialect || !dialect->providesObjectAccess() || !dialect->evmVersion().supportsReturndata()
	)
		return;

	CallGraph callGraph = CallGraphGenerator::callGraph(_ast);
	auto sideEffects = SideEffectsPropagator::sideEffects(_context.dialect, callGraph);
	CallTracer tracer{_context.dialect, sideEffects, callGraph.functionCalls};
	tracer(_ast);
	ZeroByReturndatasizeReplacer{_context.dialect, sideEffects, tracer.badFunctions(), tracer.badLoops()}(_ast);
}
