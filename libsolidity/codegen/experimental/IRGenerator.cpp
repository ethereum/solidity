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

#include <libsolidity/codegen/experimental/IRGenerator.h>

#include <libsolidity/codegen/ir/Common.h>

#include <libyul/YulStack.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/optimiser/ASTCopier.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/Whiskers.h>

#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;
using namespace solidity::util;

string IRGenerator::run(
	ContractDefinition const& _contract,
	bytes const& /*_cborMetadata*/,
	map<ContractDefinition const*, string_view const> const& /*_otherYulSources*/
) const
{

	Whiskers t(R"(
		object "<CreationObject>" {
			code {
				codecopy(0, dataoffset("<DeployedObject>"), datasize("<DeployedObject>"))
				return(0, datasize("<DeployedObject>"))
			}
			object "<DeployedObject>" {
				code {
					<code>
				}
			}
		}
	)");
	t("CreationObject", IRNames::creationObject(_contract));
	t("DeployedObject", IRNames::deployedObject(_contract));
	t("code", generate(_contract));

	return t.render();
}

string IRGenerator::generate(ContractDefinition const& _contract) const
{
	std::stringstream code;
	code << "{\n";
	if (_contract.fallbackFunction())
	{
		code << IRNames::function(*_contract.fallbackFunction()) << "()\n";
	}
	code << "revert(0,0)\n";
	code << "}\n";

	for (FunctionDefinition const* f: _contract.definedFunctions())
		code << generate(*f);

	return code.str();
}

string IRGenerator::generate(FunctionDefinition const& _function) const
{
	std::stringstream code;
	code << "function " << IRNames::function(_function) << "() {\n";
	for (auto _statement: _function.body().statements())
	{
		if (auto assembly = dynamic_cast<InlineAssembly const*>(_statement.get()))
			code << generate(*assembly) << "\n";
		else
			solUnimplemented("Unsupported statement type.");
	}
	code << "}\n";
	return code.str();
}

namespace {

struct CopyTranslate: public yul::ASTCopier
{
	CopyTranslate(
		yul::Dialect const& _dialect,
		map<yul::Identifier const*, void*> _references
	): m_dialect(_dialect), m_references(std::move(_references)) {}

	using ASTCopier::operator();

	yul::Expression operator()(yul::Identifier const& _identifier) override
	{
		// The operator() function is only called in lvalue context. In rvalue context,
		// only translate(yul::Identifier) is called.
		if (m_references.count(&_identifier))
			return translateReference(_identifier);
		else
			return ASTCopier::operator()(_identifier);
	}

	yul::YulString translateIdentifier(yul::YulString _name) override
	{
		if (m_dialect.builtin(_name))
			return _name;
		else
			return yul::YulString{"usr$" + _name.str()};
	}

	yul::Identifier translate(yul::Identifier const& _identifier) override
	{
		if (!m_references.count(&_identifier))
			return ASTCopier::translate(_identifier);

		yul::Expression translated = translateReference(_identifier);
		solAssert(holds_alternative<yul::Identifier>(translated));
		return get<yul::Identifier>(std::move(translated));
	}

private:

	/// Translates a reference to a local variable, potentially including
	/// a suffix. Might return a literal, which causes this to be invalid in
	/// lvalue-context.
	yul::Expression translateReference(yul::Identifier const&)
	{
		solUnimplemented("External references in inline assembly not implemented.");
	}

	yul::Dialect const& m_dialect;
	map<yul::Identifier const*, void*> m_references;
};

}

string IRGenerator::generate(InlineAssembly const& _assembly) const
{
	CopyTranslate bodyCopier{_assembly.dialect(), {}};
	yul::Statement modified = bodyCopier(_assembly.operations());
	solAssert(holds_alternative<yul::Block>(modified));
	return yul::AsmPrinter()(std::get<yul::Block>(modified));
}