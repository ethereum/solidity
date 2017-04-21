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
 * @author Federico Bond <federicobond@gmail.com>
 * @date 2016
 * Static analyzer and checker.
 */

#include <libsolidity/analysis/StaticAnalyzer.h>
#include <memory>
#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;


bool StaticAnalyzer::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return Error::containsOnlyWarnings(m_errors);
}

bool StaticAnalyzer::visit(ContractDefinition const& _contract)
{
	m_library = _contract.isLibrary();
	return true;
}

void StaticAnalyzer::endVisit(ContractDefinition const&)
{
	m_library = false;
}

bool StaticAnalyzer::visit(FunctionDefinition const& _function)
{
	m_nonPayablePublic = _function.isPublic() && !_function.isPayable();
	return true;
}

void StaticAnalyzer::endVisit(FunctionDefinition const&)
{
	m_nonPayablePublic = false;
}

bool StaticAnalyzer::visit(ExpressionStatement const& _statement)
{
	if (_statement.expression().annotation().isPure)
		warning(_statement.location(), "Statement has no effects.");
	return true;
}

bool StaticAnalyzer::visit(MemberAccess const& _memberAccess)
{
	if (m_nonPayablePublic && !m_library)
		if (MagicType const* type = dynamic_cast<MagicType const*>(_memberAccess.expression().annotation().type.get()))
			if (type->kind() == MagicType::Kind::Message && _memberAccess.memberName() == "value")
				warning(_memberAccess.location(), "\"msg.value\" used in non-payable function. Do you want to add the \"payable\" modifier to this function?");

	return true;
}

void StaticAnalyzer::warning(SourceLocation const& _location, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::Warning);
	*err <<
		errinfo_sourceLocation(_location) <<
		errinfo_comment(_description);

	m_errors.push_back(err);
}
