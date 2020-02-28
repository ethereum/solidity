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

#include <string>
#include <test/tools/ossfuzz/solProto.pb.h>
#include <variant>

namespace solidity::test::solprotofuzzer
{
struct ProgramInfo
{
	enum class ProgramType
	{
		LIBRARY,
		CONTRACT,
		INTERFACE
	};
	std::string m_name;
	std::string m_userDefinedTypes;
	ProgramType m_programType;
};

struct FunctionInfo
{
	enum class FunctionVisibility
	{
		PUBLIC,
		PRIVATE,
		INTERNAL,
		EXTERNAL
	};
	enum class StateMutability
	{
		PURE,
		VIEW,
		PAYABLE
	};
	std::string m_name;
	std::string m_params;
	std::string m_returns;
	FunctionVisibility m_visibility;
	StateMutability m_mutability;
	bool m_override;
	bool m_virtual;
};

class ProtoConverter
{
public:
	ProtoConverter() {}
	ProtoConverter(ProtoConverter const&) = delete;
	ProtoConverter(ProtoConverter&&) = delete;
	std::string protoToSolidity(Program const&);
private:
	using CI = std::variant<Contract const*, Interface const*>;

	std::string visit(Program const&);
	std::string visit(ContractType const&);
	std::string visit(ContractOrInterface const&);
	std::string visit(Interface const&);
	std::string visit(Contract const&);
	std::string traverseOverrides(
		Interface const&,
		bool _isOverride,
		bool _implement,
		bool _inheritedByContract,
		bool _isVirtual
	);
	std::string traverseOverrides(
		Contract const&,
		bool _isAbstract
	);
	std::string visit(Library const&);
	std::pair<std::string, std::string> visit(Block const&);
	std::pair<std::string, std::string> visit(Statement const&);
	std::pair<std::string, std::string> visit(solidity::test::abiv2fuzzer::VarDecl const&, bool _stateVar);
	std::pair<std::string, std::string> visit(IfStmt const&);
	std::string visit(ForStmt const&);
	std::string visit(SwitchStmt const&);
	std::string visit(BreakStmt const&);
	std::string visit(ContinueStmt const&);
	std::string visit(ReturnStmt const&);
	std::pair<std::string, std::string> visit(DoStmt const&);
	std::pair<std::string, std::string> visit(WhileStmt const&);
	std::string visit(Expression const&);
	std::string visit(Literal const&);
	std::string visit(BinaryOp const&);
	std::string visit(UnaryOp const&);
	std::string visit(VarRef const&);
	std::tuple<std::string, std::string, std::string> visit(
		FunctionParamsAndReturns const& _pR,
		bool _isExternal,
		std::string _programName
	);
	std::string visit(
		InterfaceFunction const&,
		unsigned _index,
		bool _isOverride,
		std::string _programName,
		bool _implement = false,
		bool _inheritedByContract = false,
		bool _isVirtual = false
	);
	std::string visit(LibraryFunction const& _func, unsigned _index, std::string _programName);
	std::string visit(
		ContractFunction const&,
		unsigned _index,
		bool _isOverride,
		bool _isAbstractContract,
		bool _isVirtual,
		bool _isImplemented,
		std::string _programName
	);
	std::tuple<std::string, std::string, std::string> visitContractHelper(CI _cOrI, std::string _programName);
	std::string visit(Modifier const&);
	static std::string functionVisibility(ContractFunction::Visibility _vis);
	static std::string functionVisibility(LibraryFunction::Visibility _vis);
	static std::string stateMutability(ContractFunction::StateMutability _mut);
	static std::string stateMutability(LibraryFunction::StateMutability _mut);
	static std::string stateMutability(InterfaceFunction::StateMutability _mut);
	static bool disallowedContractFunction(ContractFunction const& _contractFunction, bool _isVirtual);

	unsigned m_numVars = 0;
	unsigned m_numStructs = 0;
	unsigned m_numMods = 0;
	unsigned m_numContracts = 0;
	bool m_isImplemented = false;
	std::map<Interface const*, std::string> m_interfaceNameMap;
	std::map<Contract const*, std::string> m_contractNameMap;

	static auto constexpr s_interfaceFunctionPrefix = "i";
	static auto constexpr s_libraryFunctionPrefix = "l";
	static auto constexpr s_contractFunctionPrefix = "c";
	static auto constexpr s_functionPrefix = "func";
};
}