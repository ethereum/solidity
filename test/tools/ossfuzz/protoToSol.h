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
	enum MostDerivedProgram {
		CONTRACT,
		INTERFACE,
		LIBRARY
	};

	/// Variant type that points to one of contract, interface, library protobuf messages
	using CIL = std::variant<Contract const*, Interface const*, Library const*>;
	/// Variant type that points to one of contract, interface, library function protobuf
	/// messages
	using CILFunc = std::variant<ContractFunction const*, InterfaceFunction const*, LibraryFunction const*>;
	/// Variant type that points to one of contract, interface protobuf messages
	using CI = std::variant<Contract const*, Interface const*>;
	/// Variant type that points to one of contract, interface function protobuf messages
	using CIFunc = std::variant<ContractFunction const*, InterfaceFunction const*>;
	/// Tuple of contract or interface function variant and a boolean stating whether
	/// the function is implemented (true) or not and a second boolean stating whether
	/// the function is virtual (true) or not.
	using CITuple = std::tuple<CIFunc, bool, bool>;
	/// Protobuf message visitors that accept a const reference to a protobuf message
	/// type and return its solidity translation.
	std::string visit(Program const&);
	std::string visit(TestContract const&);
	std::string visit(ContractType const&);
	std::string visit(ContractOrInterface const&);
	std::string visit(Interface const& _interface);
	/// Visitor for most derived interface messages.
	/// @param _interface is a const reference to interface protobuf message
	/// @returns a 3-tuple containing Solidity translation of all base contracts
	/// this interface derives from, names of all base contracts, and the Solidity
	/// translation of this interface.
	std::tuple<std::string, std::string, std::string>
	visitMostDerivedInterface(Interface const& _interface);
	std::string visit(Contract const&);
	/// Define overrides for most derived interface.
	std::string mostDerivedInterfaceOverrides(Interface const& _interface);
	/// Define overrides for most derived contract.
	std::string mostDerivedContractOverrides(Interface const& _interface);
	std::string traverseOverrides(Contract const&);
	std::string registerAndVisitFunction(
		CIL _program,
		CILFunc _func,
		unsigned _index,
		bool _override,
		bool _virtual,
		bool _implement
	);
	std::string overrideFunction(CILFunc _function, bool _virtual, bool _implement);
	std::pair<bool, bool> contractFunctionParams(
		Contract const* _contract,
		ContractFunction const* _function
	);
	std::string visit(CILFunc _function, bool _override, bool _virtual, bool _implement);
	std::string visit(Library const&);
	std::string programName(CIL _program);
	std::string createFunctionName(CIL _program, unsigned _index);
	std::string functionName(CILFunc _function);
	void registerFunctionName(CIL _program, CILFunc _function, unsigned _index);
	std::tuple<std::string, std::string, std::string> visitProgramHelper(CIL _program);
	bool contractFunctionImplemented(Contract const* _contract, CIFunc _function);
	bool contractFunctionVirtual(Contract const* _contract, CIFunc _function);
	std::tuple<bool, bool, bool> mostDerivedContractOverrideParams();

	std::pair<bool, bool> contractFunctionOverrideParams(
		Contract const* _base,
		Contract const* _derived,
		CIFunc _baseFunc
	);
	std::tuple<std::string, std::string, std::string> pseudoRandomLibraryTest();
	std::tuple<std::string, std::string, std::string> pseudoRandomContractTest();

	bool emptyLibrary(Library const& _library)
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

	void openProgramScope(CIL _program);
	bool pseudoRandomCoinFlip()
	{
		return m_counter++ % 2 == 0;
	}
	bool mostDerivedProgramContract()
	{
		return m_mostDerivedProgram == MostDerivedProgram::CONTRACT;
	}
	bool mostDerivedProgramInterface()
	{
		return m_mostDerivedProgram == MostDerivedProgram::INTERFACE;
	}
	bool mostDerivedProgramAbstractContract()
	{
		return m_mostDerivedAbstractContract;
	}
	unsigned randomNumber();

#if 0
	static bool disallowedContractFunction(SolContractFunction const& _contractFunction, bool _isVirtual);
#endif

	unsigned m_numPrograms = 0;
	unsigned m_counter = 0;
	bool m_mostDerivedAbstractContract = false;
	bool m_libraryTest = false;
	std::shared_ptr<SolRandomNumGenerator> m_randomGen;

	MostDerivedProgram m_mostDerivedProgram = MostDerivedProgram::CONTRACT;
	/// Map whose key is pointer to protobuf interface message
	/// and whose value is its contract name
	std::map<Interface const*, std::string> m_interfaceNameMap;
	/// Map whose key is pointer to protobuf contract message
	/// and whose value is its contract name
	std::map<Contract const*, std::string> m_contractNameMap;
	/// Map whose key is library name and whose value is the
	/// number of implemented functions in it.
	std::map<std::string, unsigned> m_libraryFuncMap;
	/// Map whose key is a const pointer to protobuf contract
	/// message and whose value is a list of 3-tuples that
	/// store a const pointer to a protobuf interface or contract
	/// function belonging to the keyed contract, a boolean flag that is
	/// true when the function is implemented, false otherwise, and
	/// a second boolean flag that is true when the function is virtualized
	/// false otherwise.
	std::map<Contract const*, std::vector<CITuple>> m_contractFunctionMap;
	/// Map whose key is a const pointer to protobuf contract, interface or
	/// library function message type and whose value is the function name
	/// assigned to it.
	std::map<CILFunc, std::string> m_functionNameMap;
	std::map<CIL, std::string> m_programNameMap;
	/// Map whose key is a const pointer to protobuf contract or interface
	/// function message type and whose value is a pair of const pointer to
	/// protobuf contract or interface it belongs to and its declaration
	/// position (which is an unsigned integer that starts from 0).
	std::map<CIFunc, std::pair<CI, unsigned>> m_functionProgramMap;

	std::map<CI, std::vector<std::pair<CIFunc, std::string>>> m_programFunctionNameMap;

	std::vector<std::tuple<std::string, std::string, std::string>> m_libraryTests;
	std::vector<std::tuple<std::string, std::string, std::string>> m_contractTests;
	std::string m_libraryName;

	static auto constexpr s_interfaceFunctionPrefix = "i";
	static auto constexpr s_libraryFunctionPrefix = "l";
	static auto constexpr s_contractFunctionPrefix = "c";
	static auto constexpr s_functionPrefix = "func";
};
}