#pragma once

#include <vector>
#include <libevmasm/AssemblyItem.h>
#include <libevmasm/Instruction.h>
#include <libevmasm/RuleList.h>
#include <libevmasm/SimplificationRule.h>
#include <tuple>

#define optimizerAssert(condition) assertThrow(condition, OptimizerException, "internal optimizer exception")

namespace dev
{
namespace solidity
{
using namespace dev;
using namespace solidity;
using namespace eth;
using namespace std;

class Pattern
{
public:
	enum class Kind { Operation, Constant, Any };

public:
	Pattern(Instruction const& _instruction, vector<Pattern> const& _operands)
	: m_kind(Kind::Operation), m_hasOperationValues(true), m_instruction(_instruction), m_operands(_operands)
	{
		optimizerAssert(unsigned(instructionInfo(_instruction).args) == _operands.size());
	}

	Pattern(AssemblyItemType const& _type) : m_kind(Kind::Any), m_hasAssemblyType(true), m_assemblyType(_type)  {}

	Pattern(u256 const& _constant) : m_kind(Kind::Constant), m_hasConstant(true), m_constant(_constant) {}

	Pattern(Kind const& _kind) : m_kind(_kind) {}

	Kind kind() const { return m_kind; }

	vector<Pattern> const& operands() const
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
		assertThrow(m_hasOperationValues, OptimizerException, "invalid request for constant");
		return m_constant;
	}

	AssemblyItemType assemblyItemType() const
	{
		assertThrow(m_hasOperationValues, OptimizerException, "invalid request for assembly item type");
		return m_assemblyType;
	}

	typedef vector<AssemblyItem>::iterator AssemblyItemIterator;

	bool matches(vector<AssemblyItem> _items) const;

private:
	bool matches(Pattern _other) const;

	Kind m_kind;

	bool m_hasConstant = false;
	u256 m_constant;

	bool m_hasOperationValues = false;
	Instruction m_instruction;
	vector<Pattern> m_operands;

	bool m_hasAssemblyType = false;
	AssemblyItemType m_assemblyType;
};

class ThirdOptimizer
{
public:
	vector<AssemblyItem> optimize(vector<AssemblyItem> _items);
private:
	vector<SimplificationRule<Pattern>> m_rules;
};
}
}
