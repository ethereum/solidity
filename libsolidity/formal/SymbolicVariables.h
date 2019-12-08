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

#pragma once

#include <libsolidity/formal/SolverInterface.h>
#include <libsolidity/formal/SSAVariable.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/TypeProvider.h>
#include <memory>

namespace dev
{
namespace solidity
{
namespace smt
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
		solidity::TypePointer _type,
		solidity::TypePointer _originalType,
		std::string _uniqueName,
		EncodingContext& _context
	);
	SymbolicVariable(
		SortPointer _sort,
		std::string _uniqueName,
		EncodingContext& _context
	);

	virtual ~SymbolicVariable() = default;

	virtual Expression currentValue(solidity::TypePointer const& _targetType = TypePointer{}) const;
	std::string currentName() const;
	virtual Expression valueAtIndex(int _index) const;
	virtual std::string nameAtIndex(int _index) const;
	virtual Expression resetIndex();
	virtual Expression increaseIndex();
	virtual Expression operator()(std::vector<Expression> /*_arguments*/) const
	{
		solAssert(false, "Function application to non-function.");
	}

	unsigned index() const { return m_ssa->index(); }
	unsigned& index() { return m_ssa->index(); }

	SortPointer const& sort() const { return m_sort; }
	solidity::TypePointer const& type() const { return m_type; }
	solidity::TypePointer const& originalType() const { return m_originalType; }

protected:
	std::string uniqueSymbol(unsigned _index) const;

	/// SMT sort.
	SortPointer m_sort;
	/// Solidity type, used for size and range in number types.
	solidity::TypePointer m_type;
	/// Solidity original type, used for type conversion if necessary.
	solidity::TypePointer m_originalType;
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
		solidity::TypePointer _type,
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
		solidity::TypePointer _type,
		solidity::TypePointer _originalType,
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
		solidity::TypePointer _originalType,
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
		solidity::TypePointer _type,
		std::string _uniqueName,
		EncodingContext& _context
	);
	SymbolicFunctionVariable(
		SortPointer _sort,
		std::string _uniqueName,
		EncodingContext& _context
	);

	Expression currentValue(solidity::TypePointer const& _targetType = TypePointer{}) const override;

	// Explicit request the function declaration.
	Expression currentFunctionValue() const;

	Expression valueAtIndex(int _index) const override;

	// Explicit request the function declaration.
	Expression functionValueAtIndex(int _index) const;

	Expression resetIndex() override;
	Expression increaseIndex() override;

	Expression operator()(std::vector<Expression> _arguments) const override;

private:
	/// Creates a new function declaration.
	void resetDeclaration();

	/// Stores the current function declaration.
	Expression m_declaration;

	/// Abstract representation.
	SymbolicIntVariable m_abstract{
		TypeProvider::uint256(),
		TypeProvider::uint256(),
		m_uniqueName + "_abstract",
		m_context
	};
};

/**
 * Specialization of SymbolicVariable for Mapping
 */
class SymbolicMappingVariable: public SymbolicVariable
{
public:
	SymbolicMappingVariable(
		solidity::TypePointer _type,
		std::string _uniqueName,
		EncodingContext& _context
	);
};

/**
 * Specialization of SymbolicVariable for Array
 */
class SymbolicArrayVariable: public SymbolicVariable
{
public:
	SymbolicArrayVariable(
		solidity::TypePointer _type,
		solidity::TypePointer _originalTtype,
		std::string _uniqueName,
		EncodingContext& _context
	);

	Expression currentValue(solidity::TypePointer const& _targetType = TypePointer{}) const override;
};

/**
 * Specialization of SymbolicVariable for Enum
 */
class SymbolicEnumVariable: public SymbolicVariable
{
public:
	SymbolicEnumVariable(
		solidity::TypePointer _type,
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
		solidity::TypePointer _type,
		std::string _uniqueName,
		EncodingContext& _context
	);

	std::vector<std::shared_ptr<SymbolicVariable>> const& components()
	{
		return m_components;
	}

private:
	std::vector<std::shared_ptr<SymbolicVariable>> m_components;
};

}
}
}
