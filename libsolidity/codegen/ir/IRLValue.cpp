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

#include <libsolidity/codegen/ir/IRLValue.h>

#include <libsolidity/codegen/ir/IRGenerationContext.h>
#include <libsolidity/codegen/YulUtilFunctions.h>
#include <libsolidity/ast/AST.h>

#include <libdevcore/Whiskers.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

IRLocalVariable::IRLocalVariable(
	ostream& _code,
	IRGenerationContext& _context,
	VariableDeclaration const& _varDecl
):
	IRLValue(_code, _context, _varDecl.annotation().type),
	m_variableName(_context.localVariableName(_varDecl))
{
}

void IRLocalVariable::storeValue(string const& _value, Type const& _type) const
{
	solAssert(_type == *m_type, "Storing different types - not necessarily a problem.");
	m_code << m_variableName << " := " << _value << "\n";
}

IRStorageItem::IRStorageItem(
	ostream& _code,
	IRGenerationContext& _context,
	VariableDeclaration const& _varDecl
):
	IRLValue(_code, _context, _varDecl.annotation().type)
{
	u256 slot;
	unsigned offset;
	std::tie(slot, offset) = _context.storageLocationOfVariable(_varDecl);
	m_slot = toCompactHexWithPrefix(slot);
	m_offset = offset;
}

string IRStorageItem::retrieveValue() const
{
	if (!m_type->isValueType())
		return m_slot;
	solUnimplementedAssert(m_type->category() != Type::Category::Function, "");
	return m_context.utils().readFromStorage(*m_type, m_offset, false) + "(" + m_slot + ")";
}

void IRStorageItem::storeValue(string const& _value, Type const& _sourceType) const
{
	if (m_type->isValueType())
	{
		solAssert(m_type->storageBytes() <= 32, "Invalid storage bytes size.");
		solAssert(m_type->storageBytes() > 0, "Invalid storage bytes size.");
		solAssert(m_type->storageBytes() + m_offset <= 32, "");

		solAssert(_sourceType == *m_type, "Different type, but might not be an error.");

		m_code <<
			Whiskers("sstore(<slot>, <update>(sload(<slot>), <prepare>(<value>)))\n")
			("slot", m_slot)
			("update", m_context.utils().updateByteSliceFunction(m_type->storageBytes(), m_offset))
			("prepare", m_context.utils().prepareStoreFunction(*m_type))
			("value", _value)
			.render();
	}
	else
	{
		solAssert(
			_sourceType.category() == m_type->category(),
			"Wrong type conversation for assignment."
		);
		if (m_type->category() == Type::Category::Array)
			solUnimplementedAssert(false, "");
		else if (m_type->category() == Type::Category::Struct)
			solUnimplementedAssert(false, "");
		else
			solAssert(false, "Invalid non-value type for assignment.");
	}
}

