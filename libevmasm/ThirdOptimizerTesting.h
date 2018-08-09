#pragma once

#include <vector>
#include <libevmasm/AssemblyItem.h>
#include <libevmasm/Instruction.h>
#include <libevmasm/RuleList.h>
#include <libevmasm/SimplificationRule.h>
#include <tuple>

namespace dev
{
namespace solidity
{
using namespace dev;
using namespace solidity;
using namespace eth;
using namespace std;

class NewOptimizerPattern
{
public:
	enum class Kind { Operation, Constant, Any, Unknown };

public:
	NewOptimizerPattern(Instruction const& _instruction, vector<NewOptimizerPattern> const& _operands)
	: m_kind(Kind::Operation), m_hasOperationValues(true), m_instruction(_instruction), m_operands(_operands)
	{
		assertThrow(
			unsigned(instructionInfo(_instruction).args) == _operands.size(),
			OptimizerException,
			"number of operands passed does not match instruction"
		);
	}

	NewOptimizerPattern(Instruction const& _instruction) : NewOptimizerPattern(_instruction, {}) {}

	NewOptimizerPattern(AssemblyItemType const& _type) : m_kind(Kind::Any), m_hasAssemblyType(true), m_assemblyType(_type)  {}

	NewOptimizerPattern(u256 const& _constant) : m_kind(Kind::Constant), m_hasConstant(true), m_constant(_constant) {}

	NewOptimizerPattern(unsigned const& _constant) : NewOptimizerPattern(u256(_constant)) {}

	NewOptimizerPattern(Kind const& _kind) : m_kind(_kind) {}

	Kind kind() const { return m_kind; }

	vector<NewOptimizerPattern> const& operands() const
	{
		assertThrow(m_hasOperationValues, OptimizerException, "invalid request for operands");
		return m_operands;
	}

	Instruction instruction() const
	{
		assertThrow(m_hasOperationValues, OptimizerException, "invalid request for instruction");
		return m_instruction;
	}

	u256 constant() const
	{
		assertThrow(m_hasConstant, OptimizerException, "invalid request for constant");
		return m_constant;
	}

	AssemblyItemType assemblyItemType() const
	{
		assertThrow(m_hasAssemblyType, OptimizerException, "invalid request for assembly item type");
		return m_assemblyType;
	}

	bool matches(vector<AssemblyItem> _items) const;

	u256 d() const
	{
		assertThrow(m_hasConstant, OptimizerException, "invalid request for assembly item type");
		return m_constant;
	}

private:
	static vector<vector<AssemblyItem>> parseArguments(vector<AssemblyItem> const& _items);

	Kind m_kind;

	bool m_hasConstant = false;
	u256 m_constant;

	bool m_hasOperationValues = false;
	Instruction m_instruction;
	vector<NewOptimizerPattern> m_operands;

	bool m_hasAssemblyType = false;
	AssemblyItemType m_assemblyType;
};

class ThirdOptimizer
{
public:
	vector<AssemblyItem> optimize(vector<AssemblyItem> _items);
private:
	void addDefaultRules();
	void addRules(vector<SimplificationRule<NewOptimizerPattern>> const& _rules);
	void addRule(SimplificationRule<NewOptimizerPattern> const& _rule);
	vector<SimplificationRule<NewOptimizerPattern>> m_rules;
};
}
}
