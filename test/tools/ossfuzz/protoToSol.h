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

#include <test/tools/ossfuzz/solProto.pb.h>

#include <random>
#include <string>
#include <utility>
#include <variant>

namespace solidity::test::solprotofuzzer
{
struct SolRandomNumGenerator
{
	using RandomEngine = std::minstd_rand;

	explicit SolRandomNumGenerator(unsigned _seed): m_random(RandomEngine(_seed)) {}

	unsigned operator()()
	{
		return m_random();
	}

	RandomEngine m_random;
};

class ProtoConverter
{
public:
	ProtoConverter() {}
	ProtoConverter(ProtoConverter const&) = delete;
	ProtoConverter(ProtoConverter&&) = delete;
	std::string protoToSolidity(Program const&);
	/// @returns true if test calls a library function, false
	/// otherwise
	bool libraryTest() const;
	/// @returns name of the library under test
	std::string libraryName() const;
private:
	/// Variant type that points to one of contract, interface, library protobuf messages
	using CIL = std::variant<Contract const*, Interface const*, Library const*>;
	/// Protobuf message visitors that accept a const reference to a protobuf message
	/// type and return its solidity translation.
	std::string visit(Program const&);
	std::string visit(TestContract const&);
	std::string visit(ContractType const&);
	std::string visit(Interface const& _interface);
	std::string visit(Library const& _library);
	std::string visit(Contract const& _contract);
	std::string programName(CIL _program);
	std::tuple<std::string, std::string, std::string> pseudoRandomLibraryTest();
//	std::tuple<std::string, std::string, std::string> pseudoRandomContractTest();
	void openProgramScope(CIL _program);
	unsigned randomNumber();

	static bool emptyLibrary(Library const& _library)
	{
		return _library.funcdef_size() == 0;
	}
	bool emptyLibraryTests()
	{
		return m_libraryTests.size() == 0;
	}
	bool emptyContractTests()
	{
		return m_contractTests.size() == 0;
	}

	unsigned m_numPrograms = 0;
	bool m_libraryTest = false;
	std::shared_ptr<SolRandomNumGenerator> m_randomGen;
	std::map<CIL, std::string> m_programNameMap;
	std::vector<std::tuple<std::string, std::string, std::string>> m_libraryTests;
	std::map<std::string, std::map<std::string, std::string>> m_contractTests;
	std::string m_libraryName;

	/// Maximum number of local variables in test function
	static unsigned constexpr s_maxVars = 15;
};
}