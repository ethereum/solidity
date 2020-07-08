// SPDX-License-Identifier: GPL-3.0
/**
 * Yul dialect.
 */

#pragma once

#include <libyul/YulString.h>
#include <libyul/SideEffects.h>
#include <libyul/ControlFlowSideEffects.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <set>
#include <optional>

namespace solidity::yul
{

class YulString;
using Type = YulString;
enum class LiteralKind;
struct Literal;

struct BuiltinFunction
{
	YulString name;
	std::vector<Type> parameters;
	std::vector<Type> returns;
	SideEffects sideEffects;
	ControlFlowSideEffects controlFlowSideEffects;
	/// If true, this is the msize instruction.
	bool isMSize = false;
	/// If set, same length as the arguments, if true at index i, the i'th argument has to be a literal which means it can't be moved to variables.
	std::optional<std::vector<bool>> literalArguments;
};

struct Dialect: boost::noncopyable
{
	/// Default type, can be omitted.
	YulString defaultType;
	/// Type used for the literals "true" and "false".
	YulString boolType;
	std::set<YulString> types = {{}};

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	virtual BuiltinFunction const* builtin(YulString /*_name*/) const { return nullptr; }

	virtual BuiltinFunction const* discardFunction(YulString /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* equalityFunction(YulString /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* booleanNegationFunction() const { return nullptr; }

	/// Check whether the given type is legal for the given literal value.
	/// Should only be called if the type exists in the dialect at all.
	virtual bool validTypeForLiteral(LiteralKind _kind, YulString _value, YulString _type) const;

	virtual Literal zeroLiteralForType(YulString _type) const;

	virtual std::set<YulString> fixedFunctionNames() const { return {}; }

	Dialect() = default;
	virtual ~Dialect() = default;

	/// Old "yul" dialect. This is only used for testing.
	static Dialect const& yulDeprecated();
};

}
