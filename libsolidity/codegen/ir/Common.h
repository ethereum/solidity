// SPDX-License-Identifier: GPL-3.0
/**
 * Miscellaneous utilities for use in IR generator.
 */

#pragma once

#include <libsolidity/ast/AST.h>

#include <algorithm>
#include <string>

namespace solidity::frontend
{

/**
 * Structure that describes arity and co-arity of a Yul function, i.e. the number of its inputs and outputs.
 */
struct YulArity
{
	explicit YulArity(size_t _in, size_t _out): in(_in), out(_out) {}

	static YulArity fromType(FunctionType const& _functionType);

	bool operator==(YulArity const& _other) const { return in == _other.in && out == _other.out; }
	bool operator!=(YulArity const& _other) const { return !(*this == _other); }

	size_t in;  /// Number of input parameters
	size_t out; /// Number of output parameters
};

struct IRNames
{
	static std::string function(FunctionDefinition const& _function);
	static std::string function(VariableDeclaration const& _varDecl);
	static std::string creationObject(ContractDefinition const& _contract);
	static std::string runtimeObject(ContractDefinition const& _contract);
	static std::string internalDispatch(YulArity const& _arity);
	static std::string implicitConstructor(ContractDefinition const& _contract);
	static std::string constantValueFunction(VariableDeclaration const& _constant);
	static std::string localVariable(VariableDeclaration const& _declaration);
	static std::string localVariable(Expression const& _expression);
	/// @returns the variable name that can be used to inspect the success or failure of an external
	/// function call that was invoked as part of the try statement.
	static std::string trySuccessConditionVariable(Expression const& _expression);
	static std::string tupleComponent(size_t _i);
	static std::string zeroValue(Type const& _type, std::string const& _variableName);
};

struct IRHelpers
{
	static FunctionDefinition const* referencedFunctionDeclaration(Expression const& _expression);
};

}

// Overloading std::less() makes it possible to use YulArity as a map key. We could define operator<
// instead but such an operator would be a bit ambiguous (e.g. YulArity{2, 2} would be be greater than
// YulArity{1, 10} in lexicographical order but the latter has greater total number of inputs and outputs).
template<>
struct std::less<solidity::frontend::YulArity>
{
	bool operator() (solidity::frontend::YulArity const& _lhs, solidity::frontend::YulArity const& _rhs) const
	{
		return _lhs.in < _rhs.in || (_lhs.in == _rhs.in && _lhs.out < _rhs.out);
	}
};
