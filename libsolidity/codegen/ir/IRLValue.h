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
 * Generator for code that handles LValues.
 */

#pragma once

#include <string>
#include <ostream>

namespace dev
{
namespace solidity
{

class VariableDeclaration;
class IRGenerationContext;
class Type;

/**
 * Abstract class used to retrieve, delete and store data in LValues.
 */
class IRLValue
{
protected:
	IRLValue(std::ostream& _code, IRGenerationContext& _context, Type const* _type = nullptr):
		m_code(_code),
		m_context(_context),
		m_type(_type)
	{}

public:
	virtual ~IRLValue() = default;
	/// @returns an expression to retrieve the value of the lvalue. This might also emit
	/// code to m_code.
	virtual std::string retrieveValue() const = 0;
	/// Appends code to m_code to store the value of @a _value (should be an identifier)
	/// of type @a _type in the lvalue. Might perform type conversion.
	virtual void storeValue(std::string const& _value, Type const& _type) const = 0;

protected:
	std::ostream& m_code;
	IRGenerationContext& m_context;
	Type const* m_type;
};

class IRLocalVariable: public IRLValue
{
public:
	IRLocalVariable(
		std::ostream& _code,
		IRGenerationContext& _context,
		VariableDeclaration const& _varDecl
	);
	std::string retrieveValue() const override { return m_variableName; }
	void storeValue(std::string const& _value, Type const& _type) const override;

private:
	std::string m_variableName;
};

class IRStorageItem: public IRLValue
{
public:
	IRStorageItem(
		std::ostream& _code,
		IRGenerationContext& _context,
		VariableDeclaration const& _varDecl
	);
	IRStorageItem(
		std::ostream& _code,
		IRGenerationContext& _context,
		std::string _slot,
		unsigned _offset,
		Type const& _type
	);
	std::string retrieveValue() const override;
	void storeValue(std::string const& _value, Type const& _type) const override;

private:
	std::string m_slot;
	unsigned m_offset;
};


}
}
