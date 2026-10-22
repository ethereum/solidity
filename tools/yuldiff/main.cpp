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

/// Compares two Yul object trees structurally, treating variable and user-defined function names as equivalent
/// if they correspond 1:1 (tracked via a scoped bidirectional map). Prints a diff at the first point of divergence.

#include <tools/yuldiff/ASTComparator.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Object.h>
#include <libyul/ObjectParser.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libsmtutil/Exceptions.h>

#include <libsolutil/CommonIO.h>

#include <liblangutil/CharStream.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/EVMVersion.h>
#include <liblangutil/Scanner.h>

#include <iostream>
#include <string>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::langutil;

static std::shared_ptr<Object> parseYulFile(std::string_view const _path)
{
	std::string source = util::readFileAsString(boost::filesystem::path(std::string(_path)));
	Dialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(EVMVersion::current(), std::nullopt);
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto const charStream = std::make_shared<CharStream>(source, std::string(_path));
	auto const scanner = std::make_shared<Scanner>(*charStream);
	auto object = ObjectParser(errorReporter, dialect).parse(scanner, false);
	if (!object || errorReporter.hasErrors())
	{
		std::cerr << "Parse errors in " << _path << " (" << errors.size() << " error(s))\n";
		return nullptr;
	}
	return object;
}

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: yulASTComparator <file1.yul> <file2.yul>\n";
			return EXIT_FAILURE;
		}

		auto objA = parseYulFile(argv[1]);
		auto objB = parseYulFile(argv[2]);

		if (!objA || !objB)
		{
			std::cerr << "Aborting due to parse errors.\n";
			return EXIT_FAILURE;
		}

		Dialect const* dialect = objA->dialect();
		if (!dialect)
		{
			std::cerr << "No dialect available.\n";
			return EXIT_FAILURE;
		}

		tools::cmpast::ASTComparator cmp(*dialect);
		auto const result = cmp.compareObjects(*objA, *objB);
		if (result)
		{
			std::cout << "EQUIVALENT\n";
			return EXIT_SUCCESS;
		}
		else
		{
			auto const& mm = result.mismatch();
			std::cout << "MISMATCH\n";
			std::cout << "  at:     " << mm.path << "\n";
			std::cout << "  reason: " << mm.reason << "\n";
			if (!mm.lhs.empty())
			{
				std::cout << "\n  --- LHS ---\n" << mm.lhs << "\n";
				std::cout << "\n  --- RHS ---\n" << mm.rhs << "\n";
			}
			return EXIT_FAILURE;
		}
	}
	catch (smtutil::SMTLogicError const& _exception)
	{
		std::cerr << "SMT logic error:" << std::endl;
		std::cerr << boost::diagnostic_information(_exception);
		return 2;
	}
	catch (InternalCompilerError const& _exception)
	{
		std::cerr << "Internal compiler error:" << std::endl;
		std::cerr << boost::diagnostic_information(_exception);
		return 2;
	}
	catch (YulAssertion const& _exception)
	{
		std::cerr << "Yul assertion failed:" << std::endl;
		std::cerr << boost::diagnostic_information(_exception);
		return 2;
	}
	catch (...)
	{
		std::cerr << "Uncaught exception:" << std::endl;
		std::cerr << boost::current_exception_diagnostic_information() << std::endl;
		return 2;
	}
}
