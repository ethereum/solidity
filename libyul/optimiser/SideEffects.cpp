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
 * Specific AST walkers that collect semantical facts.
 */

#include <libyul/optimiser/SideEffects.h>

#include <libyul/optimiser/CallGraphGenerator.h>

#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libevmasm/SemanticInformation.h>
#include <libevmasm/Instruction.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

SideEffects SideEffects::join(SideEffects _s1, SideEffects _s2)
{
	return SideEffects(
		_s1.m_movable                 && _s2.m_movable,
		_s1.m_sideEffectFree          && _s2.m_sideEffectFree,
		_s1.m_sideEffectFreeIfNoMSize && _s2.m_sideEffectFreeIfNoMSize,
		_s1.m_containsMSize           || _s2.m_containsMSize,
		_s1.m_invalidatesStorage      || _s2.m_invalidatesStorage,
		_s1.m_invalidatesMemory       || _s2.m_invalidatesMemory
	);
}

SideEffects SideEffects::top()
{
	return SideEffects(false, false, false, true, true, true);
}

SideEffects SideEffects::bottom()
{
	return SideEffects(true, true, true, false, false, false);
}

SideEffects::SideEffects(BuiltinFunction const& _builtin):
	SideEffects(
		_builtin.movable,
		_builtin.sideEffectFree,
		_builtin.sideEffectFreeIfNoMSize,
		_builtin.isMSize,
		_builtin.invalidatesStorage,
		_builtin.invalidatesMemory
	) {}


SideEffects::SideEffects(dev::eth::Instruction const& _instr):
	SideEffects(
		eth::SemanticInformation::movable(_instr),
		eth::SemanticInformation::sideEffectFree(_instr),
		eth::SemanticInformation::sideEffectFreeIfNoMSize(_instr),
		_instr == eth::Instruction::MSIZE,
		eth::SemanticInformation::invalidatesStorage(_instr),
		eth::SemanticInformation::invalidatesMemory(_instr)
	) {}


SideEffectsCollector::SideEffectsCollector(Dialect const& _dialect, Block const& _ast): m_dialect(_dialect)
{
	CallGraphGenerator callGraphGenerator(_ast);
	for (YulString fname: callGraphGenerator.getUserDefinedFunctions())
		m_functionSideEffects.insert({fname, SideEffects::bottom()});
	// For each builtin (sink in the call graph), update the side effects of all the user
	// function that directly or indirectly calls the builtin.
	for (pair<YulString, set<YulString>> const& p: callGraphGenerator.getCallGraphRev())
	{
		if (BuiltinFunction const* f = m_dialect.builtin(p.first))
		{
			// TODO: We can also store builtin's side effect in m_functionSideEffects, but it may have name collision because
			//			of functional instruction for now?
			// m_functionSideEffects.insert({p.first, SideEffect(*f)});
			for (YulString userFunc: callGraphGenerator.getDirectAndIndirectCaller(p.first))
			{
				auto iter = m_functionSideEffects.find(userFunc);
				assertThrow(iter != m_functionSideEffects.end(), OptimizerException, "");
				iter->second = SideEffects::join(iter->second, SideEffects(*f));
			}
		}
		// TODO: instruction$$ is a work around to denote instruction
		else if (p.first.str().find("instruction$$") == 0 && dev::eth::c_instructions.count(p.first.str().substr(strlen("instruction$$"))))
		{
			string instrName = p.first.str().substr(strlen("instruction$$"));
			SideEffects eff = SideEffects(dev::eth::c_instructions.at(instrName));
			for (YulString userFunc: callGraphGenerator.getDirectAndIndirectCaller(p.first))
			{
				auto iter = m_functionSideEffects.find(userFunc);
				assertThrow(iter != m_functionSideEffects.end(), OptimizerException, "");
				iter->second = SideEffects::join(iter->second, eff);
			}
		}
		else
		{
			// TODO: Do something?
		}
	}
}

SideEffectsCollector::SideEffectsCollector(Dialect const& _dialect): m_dialect(_dialect) { }

void SideEffectsCollector::operator()(FunctionalInstruction const& _instr)
{
	// assertThrow(false, OptimizerException, "Encountered FunctionalInstruction " + dev::eth::instructionInfo(_instr.instruction).name);
	ASTWalker::operator()(_instr);
	m_sideEffects = SideEffects::join(m_sideEffects, SideEffects(_instr.instruction));
}

void SideEffectsCollector::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	YulString fname = _functionCall.functionName.name;
	if (BuiltinFunction const* f = m_dialect.builtin(fname))
	{
		m_sideEffects = SideEffects::join(m_sideEffects, SideEffects(*f));
	}
	else
	{
		auto iter = m_functionSideEffects.find(fname);
		// TODO: reword this exception
		// assertThrow(iter != m_functionSideEffects.end(), OptimizerException, "Function definition not found? fname = " + fname.str());

		// TODO: we now temporary mark side effect as top() if function not found. Probably should throw instead
		if (iter == m_functionSideEffects.end())
		{
			// cout << "constructor version: " << cversion << endl;
			cout << "SideEffectsCollector warning: function definition of " + fname.str() << " not found." << endl;
			m_sideEffects = SideEffects::top();
		}
		else
		{
			m_sideEffects = SideEffects::join(m_sideEffects, iter->second);
		}
	}
}

void SideEffectsCollector::operator()(FunctionDefinition const&)
{
	// skip function definitions, since we consider them as no side effects
}

SideEffects SideEffectsCollector::sideEffectsOf(Block const& _block)
{
	m_sideEffects = SideEffects::bottom();
	operator()(_block);
	return m_sideEffects;
}

SideEffects SideEffectsCollector::sideEffectsOf(Expression const& _expression)
{
	m_sideEffects = SideEffects::bottom();
	visit(_expression);
	return m_sideEffects;
}

SideEffects SideEffectsCollector::sideEffectsOf(Statement const& _statement)
{
	m_sideEffects = SideEffects::bottom();
	visit(_statement);
	return m_sideEffects;
}

