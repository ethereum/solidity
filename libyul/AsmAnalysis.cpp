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
 * Analyzer part of inline assembly.
 */

#include <libyul/AsmAnalysis.h>

#include <libyul/AST.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Utilities.h>
#include <libyul/Exceptions.h>
#include <libyul/Object.h>
#include <libyul/Scope.h>
#include <libyul/ScopeFiller.h>

#include <liblangutil/ErrorReporter.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

#include <libevmasm/Instruction.h>

#include <boost/algorithm/string.hpp>

#include <fmt/format.h>

#include <functional>

using namespace std::string_literals;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

namespace
{
inline std::string to_string(LiteralKind _kind)
{
	switch (_kind)
	{
	case LiteralKind::Number: return "number";
	case LiteralKind::Boolean: return "boolean";
	case LiteralKind::String: return "string";
	default: yulAssert(false, "");
	}
}
}

AsmAnalyzer::AsmAnalyzer(
	AsmAnalysisInfo& _analysisInfo,
	langutil::ErrorReporter& _errorReporter,
	YulNameRepository const& _nameRepository,
	ExternalIdentifierAccess::Resolver _resolver,
	std::set<std::string> _dataNames
):
	m_resolver(std::move(_resolver)),
	m_info(_analysisInfo),
	m_errorReporter(_errorReporter),
	m_nameRepository(_nameRepository),
	m_dataNames(std::move(_dataNames))
{
	if (EVMDialect const* evmDialect = m_nameRepository.evmDialect())
		m_evmVersion = evmDialect->evmVersion();
}

bool AsmAnalyzer::analyze(Block const& _block)
{
	auto watcher = m_errorReporter.errorWatcher();
	try
	{
		if (!(ScopeFiller(m_info, m_errorReporter, m_nameRepository))(_block))
			return false;

		(*this)(_block);
	}
	catch (FatalError const& error)
	{
		// NOTE: There's a cap on the number of reported errors, but watcher.ok() will work fine even if
		// we exceed it because the reporter keeps counting (it just stops adding errors to the list).
		// Note also that fact of exceeding the cap triggers a FatalError so one can get thrown even
		// if we don't make any of our errors fatal.
		yulAssert(!watcher.ok(), "Unreported fatal error: "s + error.what());
	}
	return watcher.ok();
}

AsmAnalysisInfo AsmAnalyzer::analyzeStrictAssertCorrect(Object const& _object)
{
	return analyzeStrictAssertCorrect(_object.code->nameRepository(), _object.code->block(), _object.qualifiedDataNames());
}

AsmAnalysisInfo AsmAnalyzer::analyzeStrictAssertCorrect(
	YulNameRepository const& _nameRepository,
	Block const& _block,
	std::set<std::string> const& _qualifiedDataNames)
{
	ErrorList errorList;
	langutil::ErrorReporter errors(errorList);
	AsmAnalysisInfo analysisInfo;
	bool success = yul::AsmAnalyzer(
		analysisInfo,
		errors,
		_nameRepository,
		{},
		_qualifiedDataNames
	).analyze(_block);
	yulAssert(success && !errors.hasErrors(), "Invalid assembly/yul code.");
	return analysisInfo;
}

std::vector<YulName> AsmAnalyzer::operator()(Literal const& _literal)
{
	expectValidType(_literal.type, nativeLocationOf(_literal));
	bool erroneousLiteralValue = false;
	if (_literal.kind == LiteralKind::String && !_literal.value.unlimited() && _literal.value.hint() && _literal.value.hint()->size() > 32)
	{
		erroneousLiteralValue = true;
		m_errorReporter.typeError(
			3069_error,
			nativeLocationOf(_literal),
			"String literal too long (" + std::to_string(formatLiteral(_literal, false /* _validated */ ).size()) + " > 32)"
		);
	}
	else if (_literal.kind == LiteralKind::Number && _literal.value.hint() && bigint(*_literal.value.hint()) > u256(-1))
	{
		erroneousLiteralValue = true;
		m_errorReporter.typeError(6708_error, nativeLocationOf(_literal), "Number literal too large (> 256 bits)");
	}

	if (!m_nameRepository.dialect().validTypeForLiteral(_literal.kind, _literal.value, _literal.type, m_nameRepository))
	{
		m_errorReporter.typeError(
			5170_error,
			nativeLocationOf(_literal),
			fmt::format(
				R"(Invalid type "{}" for literal "{}".)",
				m_nameRepository.requiredLabelOf(_literal.type),
				formatLiteral(_literal, false)
			)
		);
	}

	yulAssert(erroneousLiteralValue ^ validLiteral(_literal), "Invalid literal after validating it through AsmAnalyzer.");
	return {_literal.type};
}

std::vector<YulName> AsmAnalyzer::operator()(Identifier const& _identifier)
{
	yulAssert(!_identifier.name.empty());
	auto watcher = m_errorReporter.errorWatcher();
	YulName type = m_nameRepository.predefined().defaultType;

	if (m_currentScope->lookup(_identifier.name, GenericVisitor{
		[&](Scope::Variable const& _var)
		{
			if (!m_activeVariables.count(&_var))
				m_errorReporter.declarationError(
					4990_error,
					nativeLocationOf(_identifier),
					fmt::format(
						"Variable {} used before it was declared.",
						m_nameRepository.requiredLabelOf(_identifier.name)
					)
				);
			type = _var.type;
		},
		[&](Scope::Function const&)
		{
			m_errorReporter.typeError(
				6041_error,
				nativeLocationOf(_identifier),
				fmt::format("Function {} used without being called.", m_nameRepository.requiredLabelOf(_identifier.name))
			);
		}
	}))
	{
		if (m_resolver)
			// We found a local reference, make sure there is no external reference.
			m_resolver(
				_identifier,
				yul::IdentifierContext::NonExternal,
				m_currentScope->insideFunction()
			);
	}
	else
	{
		bool found = m_resolver && m_resolver(
			_identifier,
			yul::IdentifierContext::RValue,
			m_currentScope->insideFunction()
		);
		if (!found && watcher.ok())
			// Only add an error message if the callback did not do it.
			m_errorReporter.declarationError(
				8198_error,
				nativeLocationOf(_identifier),
				fmt::format("Identifier \"{}\" not found.", m_nameRepository.requiredLabelOf(_identifier.name))
			);

	}

	return {type};
}

void AsmAnalyzer::operator()(ExpressionStatement const& _statement)
{
	auto watcher = m_errorReporter.errorWatcher();
	std::vector<YulName> types = std::visit(*this, _statement.expression);
	if (watcher.ok() && !types.empty())
		m_errorReporter.typeError(
			3083_error,
			nativeLocationOf(_statement),
			"Top-level expressions are not supposed to return values (this expression returns " +
			std::to_string(types.size()) +
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

	std::set<YulName> variables;
	for (auto const& _variableName: _assignment.variableNames)
		if (!variables.insert(_variableName.name).second)
			m_errorReporter.declarationError(
				9005_error,
				nativeLocationOf(_assignment),
				fmt::format(
					"Variable {} occurs multiple times on the left-hand side of the assignment.",
					m_nameRepository.requiredLabelOf(_variableName.name)
				)
			);

	std::vector<YulName> types = std::visit(*this, *_assignment.value);

	if (types.size() != numVariables)
	{
		auto const identifierLabels = applyMap(
			_assignment.variableNames,
			[this](auto const& _identifier){ return m_nameRepository.requiredLabelOf(_identifier.name); }
		);
		m_errorReporter.declarationError(
			8678_error,
			nativeLocationOf(_assignment),
			fmt::format(
				"Variable count for assignment to \"{}\" does not match number of values ({} vs. {})",
				fmt::join(identifierLabels, ", "),
				numVariables,
				types.size()
			)
		);
	}

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
				yul::Identifier{variable.debugData, variable.name},
				yul::IdentifierContext::VariableDeclaration,
				m_currentScope->insideFunction()
			);
	for (auto const& variable: _varDecl.variables)
	{
		expectValidIdentifier(variable.name, nativeLocationOf(variable));
		expectValidType(variable.type, nativeLocationOf(variable));
	}

	if (_varDecl.value)
	{
		std::vector<YulName> types = std::visit(*this, *_varDecl.value);
		if (types.size() != numVariables)
		{
			auto const variableLabels = applyMap(
				_varDecl.variables,
				[this](auto const& _identifier){ return m_nameRepository.requiredLabelOf(_identifier.name); }
			);
			m_errorReporter.declarationError(
				3812_error,
				nativeLocationOf(_varDecl),
				fmt::format(
					"Variable count mismatch for declaration of \"{}\": {} variables and {} values.",
					fmt::join(variableLabels, ", "),
					numVariables,
					types.size()
				)
			);
		}

		for (size_t i = 0; i < _varDecl.variables.size(); ++i)
		{
			auto givenType = m_nameRepository.predefined().defaultType;
			if (i < types.size())
				givenType = types[i];
			TypedName const& variable = _varDecl.variables[i];
			if (variable.type != givenType)
				m_errorReporter.typeError(
					3947_error,
					nativeLocationOf(variable),
					fmt::format(
						R"(Assigning value of type "{}" to variable of type "{}".)",
						m_nameRepository.requiredLabelOf(givenType),
						m_nameRepository.requiredLabelOf(variable.type)
					)
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
	yulAssert(!_funDef.name.empty());
	expectValidIdentifier(_funDef.name, nativeLocationOf(_funDef));
	Block const* virtualBlock = m_info.virtualBlocks.at(&_funDef).get();
	yulAssert(virtualBlock, "");
	Scope const& varScope = scope(virtualBlock);
	for (auto const& var: _funDef.parameters + _funDef.returnVariables)
	{
		expectValidIdentifier(var.name, nativeLocationOf(var));
		expectValidType(var.type, nativeLocationOf(var));
		m_activeVariables.insert(&std::get<Scope::Variable>(varScope.identifiers.at(var.name)));
	}

	(*this)(_funDef.body);
}

std::vector<YulName> AsmAnalyzer::operator()(FunctionCall const& _funCall)
{
	yulAssert(!_funCall.functionName.name.empty());
	auto watcher = m_errorReporter.errorWatcher();
	std::vector<YulName> const* parameterTypes = nullptr;
	std::vector<YulName> const* returnTypes = nullptr;
	std::vector<std::optional<LiteralKind>> const* literalArguments = nullptr;

	if (auto const* f = m_nameRepository.builtin(_funCall.functionName.name))
	{
		if (_funCall.functionName.name == m_nameRepository.predefined().selfdestruct)
			m_errorReporter.warning(
				1699_error,
				nativeLocationOf(_funCall.functionName),
				"\"selfdestruct\" has been deprecated. "
				"Note that, starting from the Cancun hard fork, the underlying opcode no longer deletes the code and "
				"data associated with an account and only transfers its Ether to the beneficiary, "
				"unless executed in the same transaction in which the contract was created (see EIP-6780). "
				"Any use in newly deployed contracts is strongly discouraged even if the new behavior is taken into account. "
				"Future changes to the EVM might further reduce the functionality of the opcode."
			);
		else if (
			m_evmVersion.supportsTransientStorage() &&
			_funCall.functionName.name == m_nameRepository.predefined().tstore &&
			!m_errorReporter.hasError({2394})
		)
			m_errorReporter.warning(
				2394_error,
				nativeLocationOf(_funCall.functionName),
				"Transient storage as defined by EIP-1153 can break the composability of smart contracts: "
				"Since transient storage is cleared only at the end of the transaction and not at the end of the outermost call frame to the contract within a transaction, "
				"your contract may unintentionally misbehave when invoked multiple times in a complex transaction. "
				"To avoid this, be sure to clear all transient storage at the end of any call to your contract. "
				"The use of transient storage for reentrancy guards that are cleared at the end of the call is safe."
			);

		parameterTypes = &f->parameters;
		returnTypes = &f->returns;
		if (!f->definition->literalArguments.empty())
			literalArguments = &f->definition->literalArguments;

		validateInstructions(_funCall);
		m_sideEffects += f->definition->sideEffects;
	}
	else if (m_currentScope->lookup(_funCall.functionName.name, GenericVisitor{
		[&](Scope::Variable const&)
		{
			m_errorReporter.typeError(
				4202_error,
				nativeLocationOf(_funCall.functionName),
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
		if (m_resolver)
			// We found a local reference, make sure there is no external reference.
			m_resolver(
				_funCall.functionName,
				yul::IdentifierContext::NonExternal,
				m_currentScope->insideFunction()
			);
	}
	else
	{
		if (!validateInstructions(_funCall))
			m_errorReporter.declarationError(
				4619_error,
				nativeLocationOf(_funCall.functionName),
				fmt::format("Function \"{}\" not found.", m_nameRepository.requiredLabelOf(_funCall.functionName.name))
			);
		yulAssert(!watcher.ok(), "Expected a reported error.");
	}

	if (parameterTypes && _funCall.arguments.size() != parameterTypes->size())
		m_errorReporter.typeError(
			7000_error,
			nativeLocationOf(_funCall.functionName),
			fmt::format(
				"Function \"{}\" expects {} arguments but got {}.",
				m_nameRepository.requiredLabelOf(_funCall.functionName.name),
				parameterTypes->size(),
				_funCall.arguments.size()
			)
		);

	std::vector<YulName> argTypes;
	for (size_t i = _funCall.arguments.size(); i > 0; i--)
	{
		Expression const& arg = _funCall.arguments[i - 1];
		if (
			auto literalArgumentKind = (literalArguments && i <= literalArguments->size()) ?
				literalArguments->at(i - 1) :
				std::nullopt
		)
		{
			if (!std::holds_alternative<Literal>(arg))
				m_errorReporter.typeError(
					9114_error,
					nativeLocationOf(_funCall.functionName),
					"Function expects direct literals as arguments."
				);
			else if (*literalArgumentKind != std::get<Literal>(arg).kind)
				m_errorReporter.typeError(
					5859_error,
					nativeLocationOf(arg),
					"Function expects " + to_string(*literalArgumentKind) + " literal."
				);
			else if (*literalArgumentKind == LiteralKind::String)
			{
				auto const functionName = _funCall.functionName.name;
				if (functionName == m_nameRepository.predefined().datasize || functionName == m_nameRepository.predefined().dataoffset)
				{
					auto const& argumentAsLiteral = std::get<Literal>(arg);
					if (!m_dataNames.count(formatLiteral(argumentAsLiteral)))
						m_errorReporter.typeError(
							3517_error,
							nativeLocationOf(arg),
							"Unknown data object \"" + formatLiteral(argumentAsLiteral) + "\"."
						);
				}
				else if (m_nameRepository.isVerbatimFunction(functionName))
				{
					auto const& literalValue = std::get<Literal>(arg).value;
					yulAssert(literalValue.unlimited());  // verbatim literals are always unlimited
					if (literalValue.builtinStringLiteralValue().empty())
						m_errorReporter.typeError(
							1844_error,
							nativeLocationOf(arg),
							"The \"verbatim_*\" builtins cannot be used with empty bytecode."
						);
				}
				argTypes.emplace_back(expectUnlimitedStringLiteral(std::get<Literal>(arg)));
				continue;
			}
		}
		argTypes.emplace_back(expectExpression(arg));
	}
	std::reverse(argTypes.begin(), argTypes.end());

	if (parameterTypes && parameterTypes->size() == argTypes.size())
		for (size_t i = 0; i < parameterTypes->size(); ++i)
			expectType((*parameterTypes)[i], argTypes[i], nativeLocationOf(_funCall.arguments[i]));

	if (watcher.ok())
	{
		yulAssert(parameterTypes && parameterTypes->size() == argTypes.size(), "");
		yulAssert(returnTypes, "");
		return *returnTypes;
	}
	else if (returnTypes)
		return std::vector<YulName>(returnTypes->size(), m_nameRepository.predefined().defaultType);
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

	if (_switch.cases.size() == 1 && !_switch.cases[0].value)
		m_errorReporter.warning(
			9592_error,
			nativeLocationOf(_switch),
			"\"switch\" statement with only a default case."
		);

	YulName valueType = expectExpression(*_switch.expression);

	std::set<u256> cases;
	for (auto const& _case: _switch.cases)
	{
		if (_case.value)
		{
			auto watcher = m_errorReporter.errorWatcher();

			expectType(valueType, _case.value->type, nativeLocationOf(*_case.value));

			// We cannot use "expectExpression" here because *_case.value is not an
			// Expression and would be converted to an Expression otherwise.
			(*this)(*_case.value);

			/// Note: the parser ensures there is only one default case
			if (watcher.ok() && !cases.insert(_case.value->value.value()).second)
				m_errorReporter.declarationError(
					6792_error,
					nativeLocationOf(_case),
					"Duplicate case \"" +
					formatLiteral(*_case.value) +
					"\" defined."
				);
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

YulName AsmAnalyzer::expectExpression(Expression const& _expr)
{
	std::vector<YulName> types = std::visit(*this, _expr);
	if (types.size() != 1)
		m_errorReporter.typeError(
			3950_error,
			nativeLocationOf(_expr),
			"Expected expression to evaluate to one value, but got " +
			std::to_string(types.size()) +
			" values instead."
		);
	return types.empty() ? m_nameRepository.predefined().defaultType : types.front();
}

YulName AsmAnalyzer::expectUnlimitedStringLiteral(Literal const& _literal)
{
	yulAssert(_literal.kind == LiteralKind::String);
	yulAssert(m_nameRepository.dialect().validTypeForLiteral(LiteralKind::String, _literal.value, _literal.type, m_nameRepository));
	yulAssert(_literal.value.unlimited());

	return _literal.type;
}

void AsmAnalyzer::expectBoolExpression(Expression const& _expr)
{
	auto const type = expectExpression(_expr);
	if (type != m_nameRepository.predefined().boolType)
		m_errorReporter.typeError(
			1733_error,
			nativeLocationOf(_expr),
			fmt::format(
				R"(Expected a value of boolean type "{}" but got "{}")",
				m_nameRepository.requiredLabelOf(m_nameRepository.predefined().boolType),
				m_nameRepository.requiredLabelOf(type)
			)
		);
}

void AsmAnalyzer::checkAssignment(Identifier const& _variable, YulName _valueType)
{
	yulAssert(!_variable.name.empty());
	auto watcher = m_errorReporter.errorWatcher();
	YulName const* variableType = nullptr;
	bool found = false;
	if (Scope::Identifier const* var = m_currentScope->lookup(_variable.name))
	{
		if (m_resolver)
			// We found a local reference, make sure there is no external reference.
			m_resolver(
				_variable,
				yul::IdentifierContext::NonExternal,
				m_currentScope->insideFunction()
			);

		if (!std::holds_alternative<Scope::Variable>(*var))
			m_errorReporter.typeError(2657_error, nativeLocationOf(_variable), "Assignment requires variable.");
		else if (!m_activeVariables.count(&std::get<Scope::Variable>(*var)))
			m_errorReporter.declarationError(
				1133_error,
				nativeLocationOf(_variable),
				fmt::format("Variable {} used before it was declared.", m_nameRepository.requiredLabelOf(_variable.name))
			);
		else
			variableType = &std::get<Scope::Variable>(*var).type;
		found = true;
	}
	else if (m_resolver)
	{
		bool insideFunction = m_currentScope->insideFunction();
		if (m_resolver(_variable, yul::IdentifierContext::LValue, insideFunction))
		{
			found = true;
			variableType = &m_nameRepository.predefined().defaultType;
		}
	}

	if (!found && watcher.ok())
		// Only add message if the callback did not.
		m_errorReporter.declarationError(4634_error, nativeLocationOf(_variable), "Variable not found or variable not lvalue.");
	if (variableType && *variableType != _valueType)
		m_errorReporter.typeError(
			9547_error,
			nativeLocationOf(_variable),
			fmt::format(
				R"(Assigning a value of type "{}" to a variable of type "{}".)",
				m_nameRepository.requiredLabelOf(_valueType),
				m_nameRepository.requiredLabelOf(*variableType)
			)
		);

	yulAssert(!watcher.ok() || variableType, "");
}

Scope& AsmAnalyzer::scope(Block const* _block)
{
	yulAssert(m_info.scopes.count(_block) == 1, "Scope requested but not present.");
	auto scopePtr = m_info.scopes.at(_block);
	yulAssert(scopePtr, "Scope requested but not present.");
	return *scopePtr;
}

void AsmAnalyzer::expectValidIdentifier(YulName _identifier, SourceLocation const& _location)
{
	auto const label = m_nameRepository.requiredLabelOf(_identifier);
	// NOTE: the leading dot case is handled by the parser not allowing it.
	if (boost::ends_with(label, "."))
		m_errorReporter.syntaxError(
			3384_error,
			_location,
			fmt::format("\"{}\" is not a valid identifier (ends with a dot).", label)
		);

	if (label.find("..") != std::string::npos)
		m_errorReporter.syntaxError(
			7771_error,
			_location,
			fmt::format("\"{}\" is not a valid identifier (contains consecutive dots).", label)
		);

	if (m_nameRepository.dialect().reservedIdentifier(label))
		m_errorReporter.declarationError(
			5017_error,
			_location,
			fmt::format("The identifier \"{}\" is reserved and can not be used.", label)
		);
}

void AsmAnalyzer::expectValidType(YulName _type, SourceLocation const& _location)
{
	if (!m_nameRepository.isType(_type))
		m_errorReporter.typeError(
			5473_error,
			_location,
			fmt::format("\"{}\" is not a valid type (user defined types are not yet supported).", m_nameRepository.requiredLabelOf(_type))
		);
}

void AsmAnalyzer::expectType(YulName _expectedType, YulName _givenType, SourceLocation const& _location)
{
	if (_expectedType != _givenType)
		m_errorReporter.typeError(
			3781_error,
			_location,
			fmt::format(
				R"(Expected a value of type "{}" but got "{}".)",
				m_nameRepository.requiredLabelOf(_expectedType),
				m_nameRepository.requiredLabelOf(_givenType)
			)
		);
}

bool AsmAnalyzer::validateInstructions(std::string_view const _instructionIdentifier, langutil::SourceLocation const& _location)
{
	// NOTE: This function uses the default EVM version instead of the currently selected one.
	auto const& defaultDialect = EVMDialect::strictAssemblyForEVM(EVMVersion{});
	{
		auto const builtin = defaultDialect.builtin(_instructionIdentifier);
		if (builtin && builtin->instruction.has_value())
			return validateInstructions(builtin->instruction.value(), _location);
		else
			return false;
	}
}

bool AsmAnalyzer::validateInstructions(evmasm::Instruction _instr, SourceLocation const& _location)
{
	// We assume that returndatacopy, returndatasize and staticcall are either all available
	// or all not available.
	yulAssert(m_evmVersion.supportsReturndata() == m_evmVersion.hasStaticCall(), "");
	// Similarly we assume bitwise shifting and create2 go together.
	yulAssert(m_evmVersion.hasBitwiseShifting() == m_evmVersion.hasCreate2(), "");

	// These instructions are disabled in the dialect.
	yulAssert(
		_instr != evmasm::Instruction::JUMP &&
		_instr != evmasm::Instruction::JUMPI &&
		_instr != evmasm::Instruction::JUMPDEST,
	"");

	auto errorForVM = [&](ErrorId _errorId, std::string const& vmKindMessage) {
		m_errorReporter.typeError(
			_errorId,
			_location,
			fmt::format(
				R"(The "{instruction}" instruction is {kind} VMs (you are currently compiling for "{version}").)",
				fmt::arg("instruction", boost::to_lower_copy(instructionInfo(_instr, m_evmVersion).name)),
				fmt::arg("kind", vmKindMessage),
				fmt::arg("version", m_evmVersion.name())
			)
		);
	};

	// The errors below are meant to be issued when processing an undeclared identifier matching a builtin name
	// present on the default EVM version but not on the currently selected one,
	// since the other `validateInstructions()` overload uses the default EVM version.
	if (_instr == evmasm::Instruction::RETURNDATACOPY && !m_evmVersion.supportsReturndata())
		errorForVM(7756_error, "only available for Byzantium-compatible");
	else if (_instr == evmasm::Instruction::RETURNDATASIZE && !m_evmVersion.supportsReturndata())
		errorForVM(4778_error, "only available for Byzantium-compatible");
	else if (_instr == evmasm::Instruction::STATICCALL && !m_evmVersion.hasStaticCall())
		errorForVM(1503_error, "only available for Byzantium-compatible");
	else if (_instr == evmasm::Instruction::SHL && !m_evmVersion.hasBitwiseShifting())
		errorForVM(6612_error, "only available for Constantinople-compatible");
	else if (_instr == evmasm::Instruction::SHR && !m_evmVersion.hasBitwiseShifting())
		errorForVM(7458_error, "only available for Constantinople-compatible");
	else if (_instr == evmasm::Instruction::SAR && !m_evmVersion.hasBitwiseShifting())
		errorForVM(2054_error, "only available for Constantinople-compatible");
	else if (_instr == evmasm::Instruction::CREATE2 && !m_evmVersion.hasCreate2())
		errorForVM(6166_error, "only available for Constantinople-compatible");
	else if (_instr == evmasm::Instruction::EXTCODEHASH && !m_evmVersion.hasExtCodeHash())
		errorForVM(7110_error, "only available for Constantinople-compatible");
	else if (_instr == evmasm::Instruction::CHAINID && !m_evmVersion.hasChainID())
		errorForVM(1561_error, "only available for Istanbul-compatible");
	else if (_instr == evmasm::Instruction::SELFBALANCE && !m_evmVersion.hasSelfBalance())
		errorForVM(7721_error, "only available for Istanbul-compatible");
	else if (_instr == evmasm::Instruction::BASEFEE && !m_evmVersion.hasBaseFee())
		errorForVM(5430_error, "only available for London-compatible");
	else if (_instr == evmasm::Instruction::BLOBBASEFEE && !m_evmVersion.hasBlobBaseFee())
		errorForVM(6679_error, "only available for Cancun-compatible");
	else if (_instr == evmasm::Instruction::BLOBHASH && !m_evmVersion.hasBlobHash())
		errorForVM(8314_error, "only available for Cancun-compatible");
	else if (_instr == evmasm::Instruction::MCOPY && !m_evmVersion.hasMcopy())
		errorForVM(7755_error, "only available for Cancun-compatible");
	else if ((_instr == evmasm::Instruction::TSTORE || _instr == evmasm::Instruction::TLOAD) && !m_evmVersion.supportsTransientStorage())
		errorForVM(6243_error, "only available for Cancun-compatible");
	else if (_instr == evmasm::Instruction::PC)
		m_errorReporter.error(
			2450_error,
			Error::Type::SyntaxError,
			_location,
			"PC instruction is a low-level EVM feature. "
			"Because of that PC is disallowed in strict assembly."
		);
	else
		return false;

	return true;
}

bool AsmAnalyzer::validateInstructions(FunctionCall const& _functionCall)
{
	return validateInstructions(m_nameRepository.requiredLabelOf(_functionCall.functionName.name), nativeLocationOf(_functionCall.functionName));
}
