/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Lefteris Karapetsas <lefteris@ethdev.com>
 * @date 2015
 * Unit tests for Assembly Items from evmcore/Assembly.h
 */

#include <string>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <libdevcore/Log.h>
#include <libdevcore/SourceLocation.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/AST.h>
#include <libevmcore/Assembly.h>

using namespace std;
using namespace dev::eth;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

eth::AssemblyItems compileContract(const string& _sourceCode)
{
	Parser parser;
	ASTPointer<SourceUnit> sourceUnit;
	BOOST_REQUIRE_NO_THROW(sourceUnit = parser.parse(make_shared<Scanner>(CharStream(_sourceCode))));
	NameAndTypeResolver resolver({});
	resolver.registerDeclarations(*sourceUnit);
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			BOOST_REQUIRE_NO_THROW(resolver.resolveNamesAndTypes(*contract));
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			BOOST_REQUIRE_NO_THROW(resolver.checkTypeRequirements(*contract));
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			Compiler compiler;
			compiler.compileContract(*contract, map<ContractDefinition const*, bytes const*>{});

			return compiler.getRuntimeContext().getAssembly().getItems();
		}
	BOOST_FAIL("No contract found in source.");
	return AssemblyItems();
}

void checkAssemblyLocations(AssemblyItems const& _items, std::vector<SourceLocation> _locations)
{
	size_t i = 0;
	BOOST_CHECK_EQUAL(_items.size(), _locations.size());
	for (auto const& it: _items)
	{
		BOOST_CHECK_MESSAGE(it.getLocation() == _locations[i],
							std::string("Location mismatch for assembly item ") + std::to_string(i));
		++i;
	}

}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(Assembly)

BOOST_AUTO_TEST_CASE(location_test)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() returns (uint256 a)\n"
							 "  {\n"
							 "      return 16;\n"
							 "  }\n"
							 "}\n";
	std::shared_ptr<std::string const> n = make_shared<std::string>("source");
	AssemblyItems items = compileContract(sourceCode);
	std::vector<SourceLocation> locations {
		SourceLocation(0, 77, n), SourceLocation(0, 77, n),
		SourceLocation(0, 77, n), SourceLocation(0, 77, n),
		SourceLocation(0, 77, n), SourceLocation(0, 77, n),
		SourceLocation(0, 77, n), SourceLocation(0, 77, n),
		SourceLocation(), SourceLocation(),
		SourceLocation(0, 77, n), SourceLocation(0, 77, n),
		SourceLocation(), SourceLocation(), SourceLocation(),
		SourceLocation(0, 77, n), SourceLocation(0, 77, n),
		SourceLocation(0, 77, n), SourceLocation(0, 77, n),
		SourceLocation(0, 77, n), SourceLocation(0, 77, n),
		SourceLocation(0, 77, n),
		SourceLocation(18, 75, n), SourceLocation(40, 49, n),
		SourceLocation(61, 70, n), SourceLocation(61, 70, n), SourceLocation(61, 70, n),
		SourceLocation(), SourceLocation(),
		SourceLocation(61, 70, n), SourceLocation(61, 70, n), SourceLocation(61, 70, n)
		};
	checkAssemblyLocations(items, locations);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

