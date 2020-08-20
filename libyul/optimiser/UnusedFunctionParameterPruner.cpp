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
 * UnusedFunctionParameterPruner: Optimiser step that removes unused parameters from function
 * definition.
 */

#include <libyul/optimiser/UnusedFunctionParameterPruner.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/YulString.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>

#include <algorithm>
#include <optional>
#include <variant>


using namespace std;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::langutil;

namespace
{

bool anyFalse(vector<bool> const& _mask)
{
	return any_of(_mask.begin(), _mask.end(), [](bool b){ return !b; });
}

template<typename T>
vector<T> applyBooleanMask(vector<T> const& _vec, vector<bool> const& _mask)
{
	yulAssert(_vec.size() == _mask.size(), "");

	vector<T> ret;

	for (size_t i = 0; i < _mask.size(); ++i)
		if (_mask[i])
			ret.push_back(_vec[i]);

	return ret;
}

/**
 * Step 1A of UnusedFunctionParameterPruner: Find functions whose return parameters are not used in
 * the code, i.e.,
 *
 * Look at all VariableDeclaration `let x_1, ..., x_i, ..., x_n := f(y_1, ..., y_m)` and check if
 * `x_i` is unused. If all function calls to `f` has its i-th return parameter unused, we will mark
 * its i-th index so that it can be removed in later steps.
 */
class FindFunctionsWithUnusedReturns: public ASTWalker
{
public:
	explicit FindFunctionsWithUnusedReturns(
		Dialect const& _dialect,
		Block const& _block,
		map<YulString, size_t> const& _references
	):
		m_references(_references),
		m_dialect(_dialect)
	{
		(*this)(_block);
	}
	using ASTWalker::operator();
	void operator()(VariableDeclaration const& _v) override;

	/// Function name and a boolean mask, where `false` at index `i` indicates that the function
	/// return-parameter at index `i` in `FunctionDefinition::parameter` is unused at every function
	/// call-site.
	map<YulString, vector<bool>> callSiteMasks;
private:
	/// A count of all references to all Declarations.
	map<YulString, size_t> const& m_references;
	Dialect const& m_dialect;
};

/// Find functions whose return parameters are unused. Assuming that the code is in SSA form and
/// that ForLoopConditionIntoBody, ExpressionSplitter were run, all FunctionCalls with at least one
/// return value will have the form `let x_1, ..., x_n := f(y_1, ..., y_m)`
void FindFunctionsWithUnusedReturns::operator()(VariableDeclaration const& _v)
{
	if (holds_alternative<FunctionCall>(*_v.value))
	{
		YulString const& name = (get<FunctionCall>(*_v.value)).functionName.name;

		if (m_dialect.builtin(name))
			return;

		if (!callSiteMasks.count(name))
			callSiteMasks[name].resize(_v.variables.size(), false);

		for (size_t i = 0; i < _v.variables.size(); ++i)
			callSiteMasks.at(name)[i] =
				callSiteMasks.at(name)[i] || m_references.count(_v.variables[i].name);
	}
}


/**
 * Step 1B of UnusedFunctionParameterPruner:
 *
 * Find functions whose parameters (both arguments and return-parameters) are not used in its body.
 */
class FindFunctionsWithUnusedParameters
{
public:
	explicit FindFunctionsWithUnusedParameters(
		Dialect const& _dialect,
		Block const& _block,
		map<YulString, size_t> const& _references,
		map<YulString, vector<bool>>& _callSiteMasks
	):
		callSiteMasks(_callSiteMasks),
		m_references(_references),
		m_dialect(_dialect)
	{
		for (auto const& statement: _block.statements)
			if (holds_alternative<FunctionDefinition>(statement))
				findUnusedParameters(std::get<FunctionDefinition>(statement));

		for (auto const& [functionName, mask]: callSiteMasks)
			if (anyFalse(mask))
				returnMasks[functionName] = mask;
	}


	/// Function name and a boolean mask, where `false` at index `i` indicates that the function
	/// argument at index `i` in `FunctionDefinition::parameters` is unused.
	map<YulString, vector<bool>> argumentMasks;
	/// Function name and a boolean mask, where `false` at index `i` indicates that the
	/// return-parameter at index `i` in `FunctionDefinition::returnVariables` is unused.
	map<YulString, vector<bool>> returnMasks;
	/// A set of functions that could potentially be already pruned by
	/// UnusedFunctionParameterPruner, and therefore should be skipped when applying the
	/// transformation.
	set<YulString> alreadyPruned;

private:
	/// Find functions whose arguments are not used in its body. Also, find functions whose body
	/// satisfies a heuristic about pruning.
	void findUnusedParameters(FunctionDefinition const& _f);
	/// A heuristic to determine if a function was already rewritten by UnusedFunctionParameterPruner
	bool wasPruned(FunctionDefinition const& _f);

	/// Function name and a boolean mask, where `false` at index `i` indicates that the function
	/// return-parameter at index `i` in `FunctionDefinition::parameter` is unused at every function
	/// call-site.
	map<YulString, vector<bool>>& callSiteMasks;
	/// A count of all references to all Declarations.
	map<YulString, size_t> const& m_references;
	Dialect const& m_dialect;
};

void FindFunctionsWithUnusedParameters::findUnusedParameters(FunctionDefinition const& _f)
{
	if (wasPruned(_f))
	{
		alreadyPruned.insert(_f.name);
		return;
	}

	auto used = [&](auto v) -> bool { return m_references.count(v.name); };

	vector<bool> _argumentMask = applyMap(_f.parameters, used);
	if (anyFalse(_argumentMask))
		argumentMasks[_f.name] = move(_argumentMask);

	if (!callSiteMasks.count(_f.name))
		callSiteMasks[_f.name].resize(_f.returnVariables.size(), true);

	for (size_t i = 0; i < _f.returnVariables.size(); ++i)
		if (!used(_f.returnVariables[i]))
			callSiteMasks.at(_f.name)[i] = false;
}


bool FindFunctionsWithUnusedParameters::wasPruned(FunctionDefinition const& _f)
{
	// We skip the function body if it
	// 1. is empty, or
	// 2. is a single statement that is an assignment statement whose value is a non-builtin
	//    function call, or
	// 3. is a single expression-statement that is a non-builtin function call.
	// The above cases are simple enough so that the inliner alone can remove the parameters.
	if (_f.body.statements.empty())
		return true;
	if (_f.body.statements.size() == 1)
	{
		Statement const& e = _f.body.statements[0];
		if (holds_alternative<Assignment>(e))
		{
			if (holds_alternative<FunctionCall>(*get<Assignment>(e).value))
			{
				FunctionCall c = get<FunctionCall>(*get<Assignment>(e).value);
				if (!m_dialect.builtin(c.functionName.name))
					return true;
			}
		}
		else if (holds_alternative<ExpressionStatement>(e))
			if (holds_alternative<FunctionCall>(get<ExpressionStatement>(e).expression))
			{
				FunctionCall c = get<FunctionCall>(get<ExpressionStatement>(e).expression);
				if (!m_dialect.builtin(c.functionName.name))
					return true;
			}
	}

	return false;
}

/**
 * Step 3 of UnusedFunctionParameterPruner: introduce a new function in the block with body of
 * the old one. Replace the body of the old one with a function call to the new one with reduced
 * parameters.
 *
 * For example: introduce a new function `f` with the same the body as `f_1`, but with reduced
 * parameters, i.e., `function f() -> y { y := 1 }`. Now replace the body of `f_1` with a call to
 * `f`, i.e., `f_1(x) -> y { y := f() }`.
 */
class AddPrunedFunction
{
public:
	explicit AddPrunedFunction(
		NameDispenser& _nameDispenser,
		map<YulString, vector<bool>> const& _argumentMask,
		map<YulString, vector<bool>> const& _returnMask,
		map<YulString, YulString> const&  _translations,
		map<YulString, size_t> const& _references
	):
		m_references(_references),
		m_nameDispenser(_nameDispenser),
		m_argumentMask(_argumentMask),
		m_returnMask(_returnMask),
		m_translations(_translations),
		m_inverseTranslations(invertMap(m_translations))
	{}

	void operator()(Block& _block)
	{
		iterateReplacing(_block.statements, [&](Statement& _s) -> optional<vector<Statement>>
		{
			if (holds_alternative<FunctionDefinition>(_s))
			{
				FunctionDefinition& old = get<FunctionDefinition>(_s);
				if (m_inverseTranslations.count(old.name))
					return addFunction(old);
			}

			return nullopt;
		});
	}

private:
	vector<Statement> addFunction(FunctionDefinition& _old);

	/// A count of all references to all Declarations.
	map<YulString, size_t> const& m_references;

	NameDispenser& m_nameDispenser;

	/// Function name and a boolean mask, where `false` at index `i` indicates that the function
	/// argument at index `i` in `FunctionDefinition::parameters` is unused.
	map<YulString, vector<bool>> const& m_argumentMask;
	/// Function name and a boolean mask, where `false` at index `i` indicates that the
	/// return-parameter at index `i` in `FunctionDefinition::returnVariables` is unused.
	map<YulString, vector<bool>> const& m_returnMask;

	/// A map between the 'old' name and the 'new' name of the function that gets pruned. In the
	/// above example, this will store an element with key `f` and value `f_1`.
	map<YulString, YulString> const& m_translations;
	/// Inverse-Map of m_translations. In the above example, this will store an element with key
	/// `f_1` and value `f`.
	map<YulString, YulString> m_inverseTranslations;
};

vector<Statement> AddPrunedFunction::addFunction(FunctionDefinition& _old)
{
	auto generateName = [&](TypedName t)
	{
		return TypedName{
			t.location,
			m_nameDispenser.newName(t.name),
			t.type
		};
	};

	SourceLocation loc = _old.location;
	YulString newName = m_inverseTranslations.at(_old.name);
	TypedNameList functionArguments;
	TypedNameList returnVariables;
	TypedNameList renamedParameters = applyMap(_old.parameters, generateName);
	TypedNameList reducedRenamedParameters;
	TypedNameList renamedReturnVariables = applyMap(_old.returnVariables, generateName);
	TypedNameList reducedRenamedReturnVariables;

	if (m_argumentMask.count(newName))
	{
		vector<bool> const& mask = m_argumentMask.at(newName);
		functionArguments = applyBooleanMask(_old.parameters, mask);
		reducedRenamedParameters = applyBooleanMask(renamedParameters, mask);
	}
	else
	{
		functionArguments = _old.parameters;
		reducedRenamedParameters = renamedParameters;
	}

	if (m_returnMask.count(newName))
	{
		vector<bool> const& mask = m_returnMask.at(newName);
		returnVariables = applyBooleanMask(_old.returnVariables, mask);
		reducedRenamedReturnVariables = applyBooleanMask(renamedReturnVariables, mask);

		// Declare the missing variables on top of the function, if the variable was used, according
		// to `m_references`.
		VariableDeclaration v{loc, {}, nullptr};
		for (size_t i = 0; i < mask.size(); ++i)
			if (!mask[i] && m_references.count(_old.returnVariables[i].name))
				v.variables.emplace_back(_old.returnVariables[i]);

		if (!v.variables.empty())
		{
			Block block{loc, {}};
			block.statements.emplace_back(move(v));
			block.statements += move(_old.body.statements);
			swap(_old.body, block);
		}
	}
	else
	{
		returnVariables = _old.returnVariables;
		reducedRenamedReturnVariables = renamedReturnVariables;
	}

	FunctionDefinition newFunction{
		loc,
		move(newName),
		move(functionArguments),
		move(returnVariables),
		{loc, {}} // body
	};

	swap(newFunction.body, _old.body);
	swap(_old.parameters, renamedParameters);
	swap(_old.returnVariables, renamedReturnVariables);

	FunctionCall call{loc, Identifier{loc, newFunction.name}, {}};
	for (auto const& p: reducedRenamedParameters)
		call.arguments.emplace_back(Identifier{loc, p.name});

	// Replace the body of `f_1` by an assignment which calls `f`, i.e.,
	// `return_parameters = f(reduced_parameters)`
	if (!newFunction.returnVariables.empty())
	{
		Assignment assignment;
		assignment.location = loc;

		// The LHS of the assignment.
		for (auto const& r: reducedRenamedReturnVariables)
			assignment.variableNames.emplace_back(Identifier{loc, r.name});

		assignment.value = make_unique<Expression>(move(call));

		_old.body.statements.emplace_back(move(assignment));
	}
	else
		_old.body.statements.emplace_back(ExpressionStatement{loc, move(call)});

	return make_vector<Statement>(move(newFunction), move(_old));
}

} // anonymous namespace

void UnusedFunctionParameterPruner::run(OptimiserStepContext& _context, Block& _block)
{
	map<YulString, size_t> references =  ReferencesCounter::countReferences(_block);

	FindFunctionsWithUnusedReturns findReturns{_context.dialect, _block, references};
	FindFunctionsWithUnusedParameters find{_context.dialect, _block, references, findReturns.callSiteMasks};

	set<YulString> functionsWithUnusedArguments = util::keys(find.argumentMasks);
	set<YulString> functionsWithUnusedReturns = util::keys(find.returnMasks);
	set<YulString> namesToFree =
		functionsWithUnusedArguments + functionsWithUnusedReturns - find.alreadyPruned;

	// Step 2 of UnusedFunctionParameterPruner: replace all references to functions with unused
	// parameters with a new name.
	NameDisplacer replace{_context.dispenser, namesToFree};
	replace(_block);

	AddPrunedFunction add{
		_context.dispenser,
		find.argumentMasks,
		find.returnMasks,
		replace.translations(),
		references
	};
	add(_block);
}
