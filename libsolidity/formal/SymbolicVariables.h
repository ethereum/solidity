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

#pragma once

#include <libsolidity/formal/SSAVariable.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libsmtutil/SolverInterface.h>

#include <map>
#include <memory>

namespace solidity::frontend::smt
{

class EncodingContext;
class Type;

/**
 * This abstract class represents the symbolic version of a program variable.
 */
class SymbolicVariable
{
public:
	SymbolicVariable(
		frontend::Type const* _type,
		frontend::Type const* _originalType,
		std::string _uniqueName,
		EncodingContext& _context
	);
	SymbolicVariable(
		smtutil::SortPointer _sort,
		std::string _uniqueName,
		EncodingContext& _context
	);

	SymbolicVariable(SymbolicVariable&&) = default;

	virtual ~SymbolicVariable() = default;

	virtual smtutil::Expression currentValue(frontend::Type const* _targetType = nullptr) const;
	std::string currentName() const;
	virtual smtutil::Expression valueAtIndex(unsigned _index) const;
	virtual std::string nameAtIndex(unsigned _index) const;
	virtual smtutil::Expression resetIndex();
	virtual smtutil::Expression setIndex(unsigned _index);
	virtual smtutil::Expression increaseIndex();
	virtual smtutil::Expression operator()(std::vector<smtutil::Expression> /*_arguments*/) const
	{
		solAssert(false, "Function application to non-function.");
	}

	unsigned index() const { return m_ssa->index(); }
	unsigned& index() { return m_ssa->index(); }

	smtutil::SortPointer const& sort() const { return m_sort; }
	frontend::Type const* type() const { return m_type; }
	frontend::Type const* originalType() const { return m_originalType; }

protected:
	std::string uniqueSymbol(unsigned _index) const;

	/// SMT sort.
	smtutil::SortPointer m_sort;
	/// Solidity type, used for size and range in number types.
	frontend::Type const* m_type;
	/// Solidity original type, used for type conversion if necessary.
	frontend::Type const* m_originalType;
	std::string m_uniqueName;
	EncodingContext& m_context;
	std::unique_ptr<SSAVariable> m_ssa;
};

/**
 * Specialization of SymbolicVariable for Bool
 */
class SymbolicBoolVariable: public SymbolicVariable
{
public:
	SymbolicBoolVariable(
		frontend::Type const* _type,
		std::string _uniqueName,
		EncodingContext& _context
	);
};

/**
 * Specialization of SymbolicVariable for Integers
 */
class SymbolicIntVariable: public SymbolicVariable
{
public:
	SymbolicIntVariable(
		frontend::Type const* _type,
		frontend::Type const* _originalType,
		std::string _uniqueName,
		EncodingContext& _context
	);
};

/**
 * Specialization of SymbolicVariable for Address
 */
class SymbolicAddressVariable: public SymbolicIntVariable
{
public:
	SymbolicAddressVariable(
		std::string _uniqueName,
		EncodingContext& _context
	);
};

/**
 * Specialization of SymbolicVariable for FixedBytes
 */
class SymbolicFixedBytesVariable: public SymbolicIntVariable
{
public:
	SymbolicFixedBytesVariable(
		frontend::Type const* _originalType,
		unsigned _numBytes,
		std::string _uniqueName,
		EncodingContext& _context
	);
};

/**
 * Specialization of SymbolicVariable for FunctionType.
 * Besides containing a symbolic function declaration,
 * it also has an integer used as abstraction.
 * By default, the abstract representation is used when
 * values are requested, and the function declaration is
 * used when operator() is applied over arguments.
 */
class SymbolicFunctionVariable: public SymbolicVariable
{
public:
	SymbolicFunctionVariable(
		frontend::Type const* _type,
		std::string _uniqueName,
		EncodingContext& _context
	);
	SymbolicFunctionVariable(
		smtutil::SortPointer _sort,
		std::string _uniqueName,
		EncodingContext& _context
	);

	smtutil::Expression currentValue(frontend::Type const* _targetType = nullptr) const override;

	// Explicit request the function declaration.
	smtutil::Expression currentFunctionValue() const;

	smtutil::Expression valueAtIndex(unsigned _index) const override;

	// Explicit request the function declaration.
	smtutil::Expression functionValueAtIndex(unsigned _index) const;

	smtutil::Expression resetIndex() override;
	smtutil::Expression setIndex(unsigned _index) override;
	smtutil::Expression increaseIndex() override;

	smtutil::Expression operator()(std::vector<smtutil::Expression> _arguments) const override;

private:
	/// Creates a new function declaration.
	void resetDeclaration();

	/// Stores the current function declaration.
	smtutil::Expression m_declaration;

	/// Abstract representation.
	SymbolicIntVariable m_abstract{
		TypeProvider::uint256(),
		TypeProvider::uint256(),
		m_uniqueName + "_abstract",
		m_context
	};
};

/**
 * Specialization of SymbolicVariable for Enum
 */
class SymbolicEnumVariable: public SymbolicVariable
{
public:
	SymbolicEnumVariable(
		frontend::Type const* _type,
		std::string _uniqueName,
		EncodingContext& _context
	);
};

/**
 * Specialization of SymbolicVariable for Tuple
 */
class SymbolicTupleVariable: public SymbolicVariable
{
public:
	SymbolicTupleVariable(
		frontend::Type const* _type,
		std::string _uniqueName,
		EncodingContext& _context
	);
	SymbolicTupleVariable(
		smtutil::SortPointer _sort,
		std::string _uniqueName,
		EncodingContext& _context
	);

	smtutil::Expression currentValue(frontend::Type const* _targetType = nullptr) const override;

	std::vector<smtutil::SortPointer> const& components() const;
	smtutil::Expression component(
		size_t _index,
		frontend::Type const* _fromType = nullptr,
		frontend::Type const* _toType = nullptr
	) const;
};

/**
 * Specialization of SymbolicVariable for Array
 */
class SymbolicArrayVariable: public SymbolicVariable
{
public:
	SymbolicArrayVariable(
		frontend::Type const* _type,
		frontend::Type const* _originalTtype,
		std::string _uniqueName,
		EncodingContext& _context
	);
	SymbolicArrayVariable(
		smtutil::SortPointer _sort,
		std::string _uniqueName,
		EncodingContext& _context
	);

	SymbolicArrayVariable(SymbolicArrayVariable&&) = default;

	smtutil::Expression currentValue(frontend::Type const* _targetType = nullptr) const override;
	smtutil::Expression valueAtIndex(unsigned _index) const override;
	smtutil::Expression resetIndex() override { SymbolicVariable::resetIndex(); return m_pair.resetIndex(); }
	smtutil::Expression setIndex(unsigned _index) override { SymbolicVariable::setIndex(_index); return m_pair.setIndex(_index); }
	smtutil::Expression increaseIndex() override { SymbolicVariable::increaseIndex(); return m_pair.increaseIndex(); }
	smtutil::Expression elements() const;
	smtutil::Expression length() const;

	smtutil::SortPointer tupleSort() { return m_pair.sort(); }

private:
	SymbolicTupleVariable m_pair;
};

/**
 * Specialization of SymbolicVariable for Struct.
 */
class SymbolicStructVariable: public SymbolicVariable
{
public:
	SymbolicStructVariable(
		frontend::Type const* _type,
		std::string _uniqueName,
		EncodingContext& _context
	);

	/// @returns the symbolic expression representing _member.
	smtutil::Expression member(std::string const& _member) const;

	/// @returns the symbolic expression representing this struct
	/// with field _member updated.
	smtutil::Expression assignMember(std::string const& _member, smtutil::Expression const& _memberValue);

	/// @returns the symbolic expression representing this struct
	/// with all fields updated with the given values.
	smtutil::Expression assignAllMembers(std::vector<smtutil::Expression> const& _memberValues);

private:
	std::map<std::string, unsigned> m_memberIndices;
};



}
