// SPDX-License-Identifier: GPL-3.0
/**
 * Optimisation stage that replaces variables by their most recently assigned expressions.
 */

#include <libyul/optimiser/Rematerialiser.h>

#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void Rematerialiser::run(Dialect const& _dialect, Block& _ast, set<YulString> _varsToAlwaysRematerialize)
{
	Rematerialiser{_dialect, _ast, std::move(_varsToAlwaysRematerialize)}(_ast);
}

void Rematerialiser::run(
	Dialect const& _dialect,
	FunctionDefinition& _function,
	set<YulString> _varsToAlwaysRematerialize
)
{
	Rematerialiser{_dialect, _function, std::move(_varsToAlwaysRematerialize)}(_function);
}

Rematerialiser::Rematerialiser(
	Dialect const& _dialect,
	Block& _ast,
	set<YulString> _varsToAlwaysRematerialize
):
	DataFlowAnalyzer(_dialect),
	m_referenceCounts(ReferencesCounter::countReferences(_ast)),
	m_varsToAlwaysRematerialize(std::move(_varsToAlwaysRematerialize))
{
}

Rematerialiser::Rematerialiser(
	Dialect const& _dialect,
	FunctionDefinition& _function,
	set<YulString> _varsToAlwaysRematerialize
):
	DataFlowAnalyzer(_dialect),
	m_referenceCounts(ReferencesCounter::countReferences(_function)),
	m_varsToAlwaysRematerialize(std::move(_varsToAlwaysRematerialize))
{
}

void Rematerialiser::visit(Expression& _e)
{
	if (holds_alternative<Identifier>(_e))
	{
		Identifier& identifier = std::get<Identifier>(_e);
		YulString name = identifier.name;
		if (m_value.count(name))
		{
			assertThrow(m_value.at(name).value, OptimizerException, "");
			AssignedValue const& value = m_value.at(name);
			size_t refs = m_referenceCounts[name];
			size_t cost = CodeCost::codeCost(m_dialect, *value.value);
			if (
				(refs <= 1 && value.loopDepth == m_loopDepth) ||
				cost == 0 ||
				(refs <= 5 && cost <= 1 && m_loopDepth == 0) ||
				m_varsToAlwaysRematerialize.count(name)
			)
			{
				assertThrow(m_referenceCounts[name] > 0, OptimizerException, "");
				for (auto const& ref: m_references.forward[name])
					assertThrow(inScope(ref), OptimizerException, "");
				// update reference counts
				m_referenceCounts[name]--;
				for (auto const& ref: ReferencesCounter::countReferences(*value.value))
					m_referenceCounts[ref.first] += ref.second;
				_e = (ASTCopier{}).translate(*value.value);
			}
		}
	}
	DataFlowAnalyzer::visit(_e);
}

void LiteralRematerialiser::visit(Expression& _e)
{
	if (holds_alternative<Identifier>(_e))
	{
		Identifier& identifier = std::get<Identifier>(_e);
		YulString name = identifier.name;
		if (m_value.count(name))
		{
			Expression const* value = m_value.at(name).value;
			assertThrow(value, OptimizerException, "");
			if (holds_alternative<Literal>(*value))
				_e = *value;
		}
	}
	DataFlowAnalyzer::visit(_e);
}
