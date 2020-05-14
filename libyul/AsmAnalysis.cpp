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
 * Analyzer part of inline assembly.
 */

#include <libyul/AsmAnalysis.h>

#include <libyul/AsmData.h>
#include <libyul/AsmScopeFiller.h>
#include <libyul/AsmScope.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Utilities.h>
#include <libyul/Exceptions.h>
#include <libyul/Object.h>

#include <liblangutil/ErrorReporter.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>

#include <memory>
#include <functional>
#include <utility>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

bool AsmAnalyzer::analyze(Block const& _block)
{
	m_success = true;
	try
	{
		if (!(ScopeFiller(m_info, m_errorReporter))(_block))
			return false;

		(*this)(_block);
		if (!m_success)
			yulAssert(m_errorReporter.hasErrors(), "No success but no error.");
	}
	catch (FatalError const&)
	{
		// This FatalError con occur if the errorReporter has too many errors.
		yulAssert(!m_errorReporter.errors().empty(), "Fatal error detected, but no error is reported.");
	}
	return m_success && !m_errorReporter.hasErrors();
}

AsmAnalysisInfo AsmAnalyzer::analyzeStrictAssertCorrect(Dialect const& _dialect, Object const& _object)
{
	ErrorList errorList;
	langutil::ErrorReporter errors(errorList);
	AsmAnalysisInfo analysisInfo;
	bool success = yul::AsmAnalyzer(
		analysisInfo,
		errors,
		_dialect,
		{},
		_object.dataNames()
	).analyze(*_object.code);
	yulAssert(success && errorList.empty(), "Invalid assembly/yul code.");
	return analysisInfo;
}

vector<YulString> AsmAnalyzer::operator()(Literal const& _literal)
{
	expectValidType(_literal.type, _literal.location);
	if (_literal.kind == LiteralKind::String && _literal.value.str().size() > 32)
		typeError(
			_literal.location,
			"String literal too long (" + to_string(_literal.value.str().size()) + " > 32)"
		);
	else if (_literal.kind == LiteralKind::Number && bigint(_literal.value.str()) > u256(-1))
		typeError(_literal.location, "Number literal too large (> 256 bits)");
	else if (_literal.kind == LiteralKind::Boolean)
		yulAssert(_literal.value == "true"_yulstring || _literal.value == "false"_yulstring, "");

	if (!m_dialect.validTypeForLiteral(_literal.kind, _literal.value, _literal.type))
		typeError(
			_literal.location,
			"Invalid type \"" + _literal.type.str() + "\" for literal \"" + _literal.value.str() + "\"."
		);


	return {_literal.type};
}

vector<YulString> AsmAnalyzer::operator()(Identifier const& _identifier)
{
	yulAssert(!_identifier.name.empty(), "");
	size_t numErrorsBefore = m_errorReporter.errors().size();
	YulString type = m_dialect.defaultType;

	if (m_currentScope->lookup(_identifier.name, GenericVisitor{
		[&](Scope::Variable const& _var)
		{
			if (!m_activeVariables.count(&_var))
				declarationError(
					_identifier.location,
					"Variable " + _identifier.name.str() + " used before it was declared."
				);
			type = _var.type;
		},
		[&](Scope::Function const&)
		{
			typeError(
				_identifier.location,
				"Function " + _identifier.name.str() + " used without being called."
			);
		}
	}))
	{
	}
	else
	{
		bool found = false;
		if (m_resolver)
		{
			bool insideFunction = m_currentScope->insideFunction();
			size_t stackSize = m_resolver(_identifier, yul::IdentifierContext::RValue, insideFunction);
			if (stackSize != size_t(-1))
			{
				found = true;
				yulAssert(stackSize == 1, "Invalid stack size of external reference.");
			}
		}
		if (!found)
		{
			// Only add an error message if the callback did not do it.
			if (numErrorsBefore == m_errorReporter.errors().size())
				declarationError(_identifier.location, "Identifier not found.");
			m_success = false;
		}
	}

	return {type};
}

void AsmAnalyzer::operator()(ExpressionStatement const& _statement)
{
	vector<YulString> types = std::visit(*this, _statement.expression);
	if (m_success && !types.empty())
		typeError(_statement.location,
			"Top-level expressions are not supposed to return values (this expression returns " +
			to_string(types.size()) +
			" value" +
			(types.size() == 1 ? "" : "s") +
			"). Use ``pop()`` or assign them."
		);
}

void AsmAnalyzer::operator()(Assignment const& _assignment)
{
	yulAssert(_assignment.value, "");
	size_t const numVariables = _assignment.variableNames.size();
	yulAssert(numVariables >= 1, "");

	vector<YulString> types = std::visit(*this, *_assignment.value);

	if (types.size() != numVariables)
		declarationError(
			_assignment.location,
			"Variable count does not match number of values (" +
			to_string(numVariables) +
			" vs. " +
			to_string(types.size()) +
			")"
		);

	for (size_t i = 0; i < numVariables; ++i)
		if (i < types.size())
			checkAssignment(_assignment.variableNames[i], types[i]);
}

void AsmAnalyzer::operator()(VariableDeclaration const& _varDecl)
{
	size_t const numVariables = _varDecl.variables.size();
	if (m_resolver)
		for (auto const& variable: _varDecl.variables)
			// Call the resolver for variable declarations to allow it to raise errors on shadowing.
			m_resolver(
				yul::Identifier{variable.location, variable.name},
				yul::IdentifierContext::VariableDeclaration,
				m_currentScope->insideFunction()
			);
	for (auto const& variable: _varDecl.variables)
		expectValidType(variable.type, variable.location);

	if (_varDecl.value)
	{
		vector<YulString> types = std::visit(*this, *_varDecl.value);
		if (types.size() != numVariables)
			declarationError(_varDecl.location,
				"Variable count mismatch: " +
				to_string(numVariables) +
				" variables and " +
				to_string(types.size()) +
				" values."
			);

		for (size_t i = 0; i < _varDecl.variables.size(); ++i)
		{
			YulString givenType = m_dialect.defaultType;
			if (i < types.size())
				givenType = types[i];
			TypedName const& variable = _varDecl.variables[i];
			if (variable.type != givenType)
				typeError(
					variable.location,
					"Assigning value of type \"" + givenType.str() + "\" to variable of type \"" + variable.type.str() + "."
				);
		}
	}

	for (TypedName const& variable: _varDecl.variables)
		m_activeVariables.insert(&std::get<Scope::Variable>(
			m_currentScope->identifiers.at(variable.name))
		);
}

void AsmAnalyzer::operator()(FunctionDefinition const& _funDef)
{
	yulAssert(!_funDef.name.empty(), "");
	Block const* virtualBlock = m_info.virtualBlocks.at(&_funDef).get();
	yulAssert(virtualBlock, "");
	Scope& varScope = scope(virtualBlock);
	for (auto const& var: _funDef.parameters + _funDef.returnVariables)
	{
		expectValidType(var.type, var.location);
		m_activeVariables.insert(&std::get<Scope::Variable>(varScope.identifiers.at(var.name)));
	}

	(*this)(_funDef.body);
}

vector<YulString> AsmAnalyzer::operator()(FunctionCall const& _funCall)
{
	yulAssert(!_funCall.functionName.name.empty(), "");
	vector<YulString> const* parameterTypes = nullptr;
	vector<YulString> const* returnTypes = nullptr;
	vector<bool> const* needsLiteralArguments = nullptr;

	if (BuiltinFunction const* f = m_dialect.builtin(_funCall.functionName.name))
	{
		parameterTypes = &f->parameters;
		returnTypes = &f->returns;
		if (f->literalArguments)
			needsLiteralArguments = &f->literalArguments.value();
	}
	else if (!m_currentScope->lookup(_funCall.functionName.name, GenericVisitor{
		[&](Scope::Variable const&)
		{
			typeError(
				_funCall.functionName.location,
				"Attempt to call variable instead of function."
			);
		},
		[&](Scope::Function const& _fun)
		{
			parameterTypes = &_fun.arguments;
			returnTypes = &_fun.returns;
		}
	}))
	{
		if (!warnOnInstructions(_funCall.functionName.name.str(), _funCall.functionName.location))
			declarationError(_funCall.functionName.location, "Function not found.");
		m_success = false;
	}
	if (parameterTypes && _funCall.arguments.size() != parameterTypes->size())
		typeError(
			_funCall.functionName.location,
			"Function expects " +
			to_string(parameterTypes->size()) +
			" arguments but got " +
			to_string(_funCall.arguments.size()) + "."
		);

	vector<YulString> argTypes;
	for (size_t i = _funCall.arguments.size(); i > 0; i--)
	{
		Expression const& arg = _funCall.arguments[i - 1];

		argTypes.emplace_back(expectExpression(arg));

		if (needsLiteralArguments && (*needsLiteralArguments)[i - 1])
		{
			if (!holds_alternative<Literal>(arg))
				typeError(
					_funCall.functionName.location,
					"Function expects direct literals as arguments."
				);
			else if (
				_funCall.functionName.name.str() == "datasize" ||
				_funCall.functionName.name.str() == "dataoffset"
			)
				if (!m_dataNames.count(std::get<Literal>(arg).value))
					typeError(
						_funCall.functionName.location,
						"Unknown data object \"" + std::get<Literal>(arg).value.str() + "\"."
					);
		}
	}
	std::reverse(argTypes.begin(), argTypes.end());

	if (parameterTypes && parameterTypes->size() == argTypes.size())
		for (size_t i = 0; i < parameterTypes->size(); ++i)
			expectType((*parameterTypes)[i], argTypes[i], locationOf(_funCall.arguments[i]));

	if (m_success)
	{
		yulAssert(parameterTypes && parameterTypes->size() == argTypes.size(), "");
		yulAssert(returnTypes, "");
		return *returnTypes;
	}
	else if (returnTypes)
		return vector<YulString>(returnTypes->size(), m_dialect.defaultType);
	else
		return {};
}

void AsmAnalyzer::operator()(If const& _if)
{
	expectBoolExpression(*_if.condition);

	(*this)(_if.body);
}

void AsmAnalyzer::operator()(Switch const& _switch)
{
	yulAssert(_switch.expression, "");

	YulString valueType = expectExpression(*_switch.expression);

	set<u256> cases;
	for (auto const& _case: _switch.cases)
	{
		if (_case.value)
		{
			expectType(valueType, _case.value->type, _case.value->location);

			// We cannot use "expectExpression" here because *_case.value is not an
			// Expression and would be converted to an Expression otherwise.
			(*this)(*_case.value);

			/// Note: the parser ensures there is only one default case
			if (m_success && !cases.insert(valueOfLiteral(*_case.value)).second)
				declarationError(_case.location, "Duplicate case defined.");
		}

		(*this)(_case.body);
	}
}

void AsmAnalyzer::operator()(ForLoop const& _for)
{
	yulAssert(_for.condition, "");

	Scope* outerScope = m_currentScope;

	(*this)(_for.pre);

	// The block was closed already, but we re-open it again and stuff the
	// condition, the body and the post part inside.
	m_currentScope = &scope(&_for.pre);

	expectBoolExpression(*_for.condition);
	// backup outer for-loop & create new state
	auto outerForLoop = m_currentForLoop;
	m_currentForLoop = &_for;

	(*this)(_for.body);
	(*this)(_for.post);

	m_currentScope = outerScope;
	m_currentForLoop = outerForLoop;
}

void AsmAnalyzer::operator()(Block const& _block)
{
	auto previousScope = m_currentScope;
	m_currentScope = &scope(&_block);

	for (auto const& s: _block.statements)
		std::visit(*this, s);

	m_currentScope = previousScope;
}

YulString AsmAnalyzer::expectExpression(Expression const& _expr)
{
	vector<YulString> types = std::visit(*this, _expr);
	if (types.size() != 1)
		typeError(
			locationOf(_expr),
			"Expected expression to evaluate to one value, but got " +
			to_string(types.size()) +
			" values instead."
		);
	return types.empty() ? m_dialect.defaultType : types.front();
}

void AsmAnalyzer::expectBoolExpression(Expression const& _expr)
{
	YulString type = expectExpression(_expr);
	if (type != m_dialect.boolType)
		typeError(locationOf(_expr),
			"Expected a value of boolean type \"" +
			m_dialect.boolType.str() +
			"\" but got \"" +
			type.str() +
			"\""
		);
}

void AsmAnalyzer::checkAssignment(Identifier const& _variable, YulString _valueType)
{
	yulAssert(!_variable.name.empty(), "");
	size_t numErrorsBefore = m_errorReporter.errors().size();
	YulString const* variableType = nullptr;
	bool found = false;
	if (Scope::Identifier const* var = m_currentScope->lookup(_variable.name))
	{
		// Check that it is a variable
		if (!holds_alternative<Scope::Variable>(*var))
			typeError(_variable.location, "Assignment requires variable.");
		else if (!m_activeVariables.count(&std::get<Scope::Variable>(*var)))
			declarationError(
				_variable.location,
				"Variable " + _variable.name.str() + " used before it was declared."
			);
		else
			variableType = &std::get<Scope::Variable>(*var).type;
		found = true;
	}
	else if (m_resolver)
	{
		bool insideFunction = m_currentScope->insideFunction();
		size_t variableSize = m_resolver(_variable, yul::IdentifierContext::LValue, insideFunction);
		if (variableSize != size_t(-1))
		{
			found = true;
			variableType = &m_dialect.defaultType;
			yulAssert(variableSize == 1, "Invalid stack size of external reference.");
		}
	}

	if (!found)
	{
		m_success = false;
		// Only add message if the callback did not.
		if (numErrorsBefore == m_errorReporter.errors().size())
			declarationError(_variable.location, "Variable not found or variable not lvalue.");
	}
	if (variableType && *variableType != _valueType)
		typeError(_variable.location,
			"Assigning a value of type \"" +
			_valueType.str() +
			"\" to a variable of type \"" +
			variableType->str() +
			"\"."
		);

	if (m_success)
		yulAssert(variableType, "");
}

Scope& AsmAnalyzer::scope(Block const* _block)
{
	yulAssert(m_info.scopes.count(_block) == 1, "Scope requested but not present.");
	auto scopePtr = m_info.scopes.at(_block);
	yulAssert(scopePtr, "Scope requested but not present.");
	return *scopePtr;
}

void AsmAnalyzer::expectValidType(YulString _type, SourceLocation const& _location)
{
	if (!m_dialect.types.count(_type))
		typeError(
			_location,
			"\"" + _type.str() + "\" is not a valid type (user defined types are not yet supported)."
		);
}

void AsmAnalyzer::expectType(YulString _expectedType, YulString _givenType, SourceLocation const& _location)
{
	if (_expectedType != _givenType)
		typeError(_location,
			"Expected a value of type \"" +
			_expectedType.str() +
			"\" but got \"" +
			_givenType.str() +
			"\""
		);
}

bool AsmAnalyzer::warnOnInstructions(std::string const& _instructionIdentifier, langutil::SourceLocation const& _location)
{
	auto const builtin = EVMDialect::strictAssemblyForEVM(EVMVersion{}).builtin(YulString(_instructionIdentifier));
	if (builtin)
		return warnOnInstructions(builtin->instruction.value(), _location);
	else
		return false;
}

bool AsmAnalyzer::warnOnInstructions(evmasm::Instruction _instr, SourceLocation const& _location)
{
	// We assume that returndatacopy, returndatasize and staticcall are either all available
	// or all not available.
	yulAssert(m_evmVersion.supportsReturndata() == m_evmVersion.hasStaticCall(), "");
	// Similarly we assume bitwise shifting and create2 go together.
	yulAssert(m_evmVersion.hasBitwiseShifting() == m_evmVersion.hasCreate2(), "");

	auto errorForVM = [&](string const& vmKindMessage) {
		typeError(
			_location,
			"The \"" +
			boost::to_lower_copy(instructionInfo(_instr).name)
			+ "\" instruction is " +
			vmKindMessage +
			" VMs " +
			" (you are currently compiling for \"" +
			m_evmVersion.name() +
			"\")."
		);
	};

	if ((
		_instr == evmasm::Instruction::RETURNDATACOPY ||
		_instr == evmasm::Instruction::RETURNDATASIZE
	) && !m_evmVersion.supportsReturndata())
	{
		errorForVM("only available for Byzantium-compatible");
	}
	else if (_instr == evmasm::Instruction::STATICCALL && !m_evmVersion.hasStaticCall())
	{
		errorForVM("only available for Byzantium-compatible");
	}
	else if ((
		_instr == evmasm::Instruction::SHL ||
		_instr == evmasm::Instruction::SHR ||
		_instr == evmasm::Instruction::SAR
	) && !m_evmVersion.hasBitwiseShifting())
	{
		errorForVM("only available for Constantinople-compatible");
	}
	else if (_instr == evmasm::Instruction::CREATE2 && !m_evmVersion.hasCreate2())
	{
		errorForVM("only available for Constantinople-compatible");
	}
	else if (_instr == evmasm::Instruction::EXTCODEHASH && !m_evmVersion.hasExtCodeHash())
	{
		errorForVM("only available for Constantinople-compatible");
	}
	else if (_instr == evmasm::Instruction::CHAINID && !m_evmVersion.hasChainID())
	{
		errorForVM("only available for Istanbul-compatible");
	}
	else if (_instr == evmasm::Instruction::SELFBALANCE && !m_evmVersion.hasSelfBalance())
	{
		errorForVM("only available for Istanbul-compatible");
	}
	else if (
		_instr == evmasm::Instruction::JUMP ||
		_instr == evmasm::Instruction::JUMPI ||
		_instr == evmasm::Instruction::JUMPDEST
	)
	{
		m_errorReporter.error(
			4316_error,
			Error::Type::SyntaxError,
			_location,
			"Jump instructions and labels are low-level EVM features that can lead to "
			"incorrect stack access. Because of that they are disallowed in strict assembly. "
			"Use functions, \"switch\", \"if\" or \"for\" statements instead."
		);
		m_success = false;
	}
	else
		return false;

	return true;
}

void AsmAnalyzer::typeError(SourceLocation const& _location, string const& _description)
{
	m_errorReporter.typeError(7569_error, _location, _description);
	m_success = false;
}

void AsmAnalyzer::declarationError(SourceLocation const& _location, string const& _description)
{
	m_errorReporter.declarationError(9595_error, _location, _description);
	m_success = false;
}

