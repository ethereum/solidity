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

#include <sstream>
#include "protoToSol.h"
#include "protoToAbiV2.h"

#include <libsolutil/Whiskers.h>

using namespace solidity::test::solprotofuzzer;
using namespace std;
using namespace solidity::util;

string ProtoConverter::protoToSolidity(Program const& _p)
{
	return visit(_p);
}

string ProtoConverter::visit(Program const& _p)
{
	ostringstream program;
	ostringstream contracts;

	for (auto &contract: _p.contracts())
		contracts << visit(contract);

	program << Whiskers(R"(
pragma solidity >=0.0;
pragma experimental ABIEncoderV2;
pragma experimental SMTChecker;

<contracts>
)")
	("contracts", contracts.str())
	.render();
	return program.str();
}

string ProtoConverter::visit(ContractType const& _contractType)
{
	switch (_contractType.contract_type_oneof_case())
	{
	case ContractType::kC:
		return visit(_contractType.c());
	case ContractType::kL:
		return visit(_contractType.l());
	case ContractType::kI:
		return visit(_contractType.i());
	case ContractType::CONTRACT_TYPE_ONEOF_NOT_SET:
		return "";
	}
}

string ProtoConverter::visit(ContractOrInterface const& _contractOrInterface)
{
	switch (_contractOrInterface.contract_or_interface_oneof_case())
	{
	case ContractOrInterface::kC:
		return visit(_contractOrInterface.c());
	case ContractOrInterface::kI:
		return visit(_contractOrInterface.i());
	case ContractOrInterface::CONTRACT_OR_INTERFACE_ONEOF_NOT_SET:
		return "";
	}
}

string ProtoConverter::traverseOverrides(Interface const& _interface, bool _isOverride, bool _implement, bool _inheritedByContract, bool _isVirtual)
{
	ostringstream funcs;

	for (auto ancestor = _interface.ancestors().rbegin(); ancestor != _interface.ancestors().rend(); ancestor++)
	{
		unsigned index = 0;
		m_numContracts++;

		m_numStructs = 0;

		for (auto& f: ancestor->funcdef())
		{
			string funcStr = visit(
				f,
				index++,
				true,
				m_interfaceNameMap[&*ancestor],
				_implement,
				_inheritedByContract,
				_isVirtual
			);

			if (f.override() || _isOverride)
				funcs << funcStr;
		}
		funcs << traverseOverrides(*ancestor, _isOverride, _implement, _inheritedByContract, _isVirtual);
	}
	return funcs.str();
}

string ProtoConverter::traverseOverrides(Contract const& _contract, bool _isAbstract)
{
	ostringstream funcs;
	bool isImplemented = m_isImplemented;

	for (auto ancestor = _contract.ancestors().rbegin(); ancestor != _contract.ancestors().rend(); ancestor++)
	{
		if (ancestor->contract_or_interface_oneof_case() == ContractOrInterface::CONTRACT_OR_INTERFACE_ONEOF_NOT_SET)
			continue;

		m_numContracts++;
		m_numStructs = 0;

		if (ancestor->has_c())
		{
			unsigned index = 0;
			if (_contract.abstract() && !ancestor->c().abstract())
				m_isImplemented = true;

			for (auto& f: ancestor->c().funcdef())
			{
				string funcStr = visit(
					f,
					index++,
					true,
					_isAbstract,
					f.virtualfunc() || !f.implemented(),
					m_isImplemented,
					m_contractNameMap[&ancestor->c()]
				);
				if ((f.virtualfunc() && !f.implemented() && !_isAbstract) ||
				(f.virtualfunc() && f.override()) ||
				(ancestor->c().abstract() && !f.implemented()))
					funcs << funcStr;
			}
			funcs << traverseOverrides(ancestor->c(), _isAbstract);
		}
		else if (ancestor->has_i())
		{
			unsigned index = 0;
			for (auto& f: ancestor->i().funcdef())
				funcs << visit(
					f,
					index++,
					true,
					m_interfaceNameMap[&ancestor->i()],
					true,
					true,
					true
				);
			funcs << traverseOverrides(ancestor->i(), true, true, true, true);
		}
	}
	m_isImplemented = isImplemented;
	return funcs.str();
}

tuple<string, string, string> ProtoConverter::visitContractHelper(CI _cOrI, string _programName)
{
	ostringstream ancestors;
	ostringstream funcs;
	ostringstream ancestorNames;

	string separator{};
	if (holds_alternative<Interface const*>(_cOrI))
	{
		auto interface = get<Interface const*>(_cOrI);
		for (auto &ancestor: interface->ancestors())
		{
			string ancestorStr = visit(ancestor);
			if (ancestorStr.empty())
				continue;
			ancestors << ancestorStr;
			ancestorNames << separator
						<< m_interfaceNameMap[&ancestor];
			if (separator.empty())
				separator = ", ";
		}

		unsigned wasNumContract = m_numContracts;

		// First define overridden functions
		bool overrides = interface->ancestors_size() > 0 && !ancestorNames.str().empty();
		if (overrides)
		{
			funcs << traverseOverrides(*interface, false, false, false, false);
		}

		m_numContracts = wasNumContract;
		m_numStructs = 0;

		unsigned index = 0;
		// Define non-overridden functions
		for (auto &f: interface->funcdef())
			funcs << visit(f, index++, false, _programName);
	}
	else
	{
		auto contract = get<Contract const*>(_cOrI);

		for (auto &ancestor: contract->ancestors())
		{
			string ancestorStr = visit(ancestor);
			if (ancestorStr.empty())
				continue;
			ancestors << ancestorStr;
			ancestorNames << separator
			              << (ancestor.has_c() ? m_contractNameMap[&ancestor.c()] : m_interfaceNameMap[&ancestor.i()]);
			if (separator.empty())
				separator = ", ";
		}

		unsigned wasNumContract = m_numContracts;

		// First define overridden functions
		bool overrides = contract->ancestors_size() > 0 && !ancestorNames.str().empty();
		if (overrides)
		{
			funcs << traverseOverrides(*contract, contract->abstract());
		}

		m_numContracts = wasNumContract;
		m_numStructs = 0;

		// Define non-overridden functions
		unsigned index = 0;
		for (auto &f: contract->funcdef())
			funcs << visit(
				f,
				index++,
				false,
				contract->abstract(),
				(f.virtualfunc() || (contract->abstract() && !f.implemented())),
				false,
				_programName
			);
	}
	return make_tuple(ancestors.str(), ancestorNames.str(), funcs.str());
}

string ProtoConverter::visit(Interface const& _interface)
{
	string programName{"I" + to_string(m_numContracts++)};
	m_interfaceNameMap.insert(pair(&_interface, programName));
	m_numStructs = 0;
	auto [ancestors, ancestorNames, funcs] = visitContractHelper(&_interface, programName);
	return Whiskers(R"(
<ancestors>
<programType> <programName><?inheritance> is <ancestorNames></inheritance> {
<functionDefs>
})")
		("ancestors", ancestors)
		("programType", "interface")
		("programName", programName)
		("inheritance", _interface.ancestors_size() > 0 && !ancestorNames.empty())
		("ancestorNames", ancestorNames)
		("functionDefs", funcs)
		.render();
}

string ProtoConverter::visit(Library const& _library)
{
	ostringstream funcs;
	string programName{"L" + to_string(m_numContracts++)};

	unsigned index = 0;
	m_numStructs = 0;
	for (auto &f: _library.funcdef())
		funcs << visit(f, index++, programName);

	return Whiskers(R"(
library <programName> {
<functionDefs>
})")
		("programName", programName)
		("functionDefs", funcs.str())
		.render();
}

string ProtoConverter::visit(Contract const& _contract)
{
	string programName{"C" + to_string(m_numContracts++)};
	m_contractNameMap.insert(pair(&_contract, programName));
	m_numStructs = 0;
	auto [ancestors, ancestorNames, funcs] = visitContractHelper(&_contract, programName);
	return Whiskers(R"(
<ancestors>
<?isAbstract>abstract </isAbstract><programType> <programName><?inheritance> is <ancestorNames></inheritance> {
<functionDefs>
})")
		("ancestors", ancestors)
		("isAbstract", _contract.abstract())
		("programType", "contract")
		("programName", programName)
		("inheritance", _contract.ancestors_size() > 0 && !ancestorNames.empty())
		("ancestorNames", ancestorNames)
		("functionDefs", funcs)
		.render();
}

string ProtoConverter::visit(Modifier const& _mod)
{
	return Whiskers(R"(
	modifier m<i>(<?isParams><params></isParams>) {
		<body>
		_;
	})")
		("i", to_string(m_numMods++))
		("isParams", _mod.params_size() > 0)
		("params", "")
		("body", "")
		.render();
}

tuple<string, string, string> ProtoConverter::visit(
	FunctionParamsAndReturns const& _pr,
	bool _isExternal,
	string _programName
)
{
	string paramsString;
	string typeDefsParamsString;
	unsigned structsAdded = 0;
	unsigned index = 0;
	string separator{};
	string structPrefix = _programName + "S";

	for (auto &param: _pr.params())
	{
		solidity::test::abiv2fuzzer::TypeVisitor typeVisitor(m_numStructs, 2, structPrefix);
		typeVisitor.visit(param);
		if (!typeVisitor.baseType().empty())
		{
			paramsString += Whiskers(
				R"(<sep><type><?isNonValue> <?isExternal>calldata<!isExternal>memory</isExternal></isNonValue> p<i>)")
				("type", typeVisitor.baseType())
				("isNonValue", param.type_oneof_case() == param.kNvtype)
				("isExternal", _isExternal)
				("i", to_string(index++))
				("sep", separator)
				.render();
			typeDefsParamsString += typeVisitor.structDef();
			m_numStructs += typeVisitor.numStructs();
			structsAdded += typeVisitor.numStructs();
			if (separator.empty())
				separator = ", ";
		}
	}

	separator = "";
	string returnString;
	string typeDefsReturnsString;
	for (auto &ret: _pr.returns())
	{
		solidity::test::abiv2fuzzer::TypeVisitor typeVisitor(m_numStructs, 2, structPrefix);
		typeVisitor.visit(ret);
		if (!typeVisitor.baseType().empty())
		{
			returnString += Whiskers(R"(<sep><type><?isNonValue> memory</isNonValue>)")
				("type", typeVisitor.baseType())
				("isNonValue", ret.type_oneof_case() == ret.kNvtype)
				("sep", separator)
				.render();
			typeDefsReturnsString += typeVisitor.structDef();
			m_numStructs += typeVisitor.numStructs();
			structsAdded += typeVisitor.numStructs();
			if (separator.empty())
				separator = ", ";
		}
	}
	return make_tuple(typeDefsParamsString + typeDefsReturnsString, paramsString, returnString);
}

bool ProtoConverter::disallowedContractFunction(ContractFunction const& _contractFunction, bool _isVirtual)
{
	// Private virtual functions are disallowed
	if (functionVisibility(_contractFunction.vis()) == "private" && _isVirtual)
		return true;
	// Private payable functions are disallowed
	else if (functionVisibility(_contractFunction.vis()) == "private" && stateMutability(_contractFunction.mut()) == "payable")
		return true;
	// Internal payable functions are disallowed
	else if (functionVisibility(_contractFunction.vis()) == "internal" && stateMutability(_contractFunction.mut()) == "payable")
		return true;
	return false;
}

string ProtoConverter::visit(
	ContractFunction const& _contractFunction,
	unsigned _index,
	bool _isOverride,
	bool _isAbstractContract,
	bool _isVirtual,
	bool _isImplemented,
	string _programName
)
{
	if (disallowedContractFunction(_contractFunction, _isVirtual || _contractFunction.virtualfunc()))
		return "";

	auto [structDefString, paramString, returnString] = visit(
		_contractFunction.paramandret(),
		_contractFunction.vis() == ContractFunction_Visibility_EXTERNAL,
		_programName
	);

	bool isUnimplemented = _isAbstractContract && !_contractFunction.implemented() && !_isImplemented;

	return Whiskers(R"(
<?isTypeDefs><typeDefs></isTypeDefs>

	function <functionPrefix><i>(<?isParams><params></isParams>)<?isOverride> override</isOverride><?isVirtual> virtual</isVirtual> <visibility> <stateMutability><?isMod> <modifier></isMod><?isReturn> returns (<types>)</isReturn><?isUnimplemented>;<!isUnimplemented>
	{
		<body>
	}
	</isUnimplemented>
<?isMod>
<modifierDef>
</isMod>)")
		("isTypeDefs", !_isOverride)
		("typeDefs", structDefString)
		("functionPrefix", boost::algorithm::to_lower_copy(_programName) + s_functionPrefix)
		("i", to_string(_index))
		("isParams", _contractFunction.paramandret().params_size() > 0 && !paramString.empty())
		("params", paramString)
		("isVirtual", _isVirtual)
		("isOverride", _isOverride)
		("visibility", functionVisibility(_contractFunction.vis()))
		("stateMutability", stateMutability(_contractFunction.mut()))
		("isMod", _contractFunction.has_m() && !isUnimplemented)
		("modifier", "m" + to_string(m_numMods))
		("isReturn", _contractFunction.paramandret().returns_size() > 0 && !returnString.empty())
		("types", returnString)
		("body", "")
		("isUnimplemented", isUnimplemented)
		("modifierDef", visit(_contractFunction.m()))
		.render();
}

string ProtoConverter::visit(LibraryFunction const& _libraryFunction, unsigned _index, string _programName)
{
	auto [typeDefs, params, returns] = visit(
		_libraryFunction.paramandret(),
		_libraryFunction.vis() == LibraryFunction_Visibility_EXTERNAL,
		_programName
	);

	return Whiskers(R"(
<typeDefs>

	function <functionPrefix><functionSuffix>(<?isParams><params></isParams>) <visibility> <stateMutability><?isMod> <modifier></isMod><?isReturn> returns (<types>)</isReturn>
	{
		<body>
	}

<?isMod>
<modifierDef>
</isMod>)")
		("typeDefs", typeDefs)
		("functionPrefix", s_libraryFunctionPrefix)
		("functionSuffix", to_string(_index))
		("isParams", _libraryFunction.paramandret().params_size() > 0 && !params.empty())
		("params", params)
		("visibility", functionVisibility(_libraryFunction.vis()))
		("stateMutability", stateMutability(_libraryFunction.mut()))
		("isMod", _libraryFunction.has_m())
		("modifier", "m" + to_string(m_numMods))
		("isReturn", _libraryFunction.paramandret().returns_size() > 0 && !returns.empty())
		("types", returns)
		("body", "")
		("modifierDef", visit(_libraryFunction.m()))
		.render();
}

string ProtoConverter::visit(
	InterfaceFunction const& _interfaceFunction,
	unsigned _index,
	bool _isOverride,
	string _programName,
	bool _implement,
	bool _inheritedByContract,
	bool _isVirtual
)
{
	auto [typeDefs, params, returns] = visit(
		_interfaceFunction.paramandret(),
		true,
		_programName
	);

	return Whiskers(R"(
<?isTypeDefs><typeDefs></isTypeDefs>

	function <functionPrefix><functionSuffix>(<?isParams><params></isParams>) external <stateMutability><?isOverride> override</isOverride><?isVirtual> virtual</isVirtual><?isReturn> returns (<types>)</isReturn><?isImplemented> <block><!isImplemented><?inheritedByContract> virtual;<!inheritedByContract>;</inheritedByContract></isImplemented>
		)")
		("isTypeDefs", !_isOverride)
		("typeDefs", typeDefs)
		("functionPrefix", boost::algorithm::to_lower_copy(_programName) + s_functionPrefix)
		("functionSuffix", to_string(_index))
		("isParams", _interfaceFunction.paramandret().params_size() > 0 && !params.empty())
		("params", params)
		("stateMutability", stateMutability(_interfaceFunction.mut()))
		("isOverride", _isOverride)
		("isVirtual", _isVirtual)
		("isReturn", _interfaceFunction.paramandret().returns_size() > 0 && !returns.empty())
		("types", returns)
		("isImplemented", _implement)
		("inheritedByContract", _inheritedByContract)
		("block", "{}")
		.render();
}

pair<string, string> ProtoConverter::visit(Block const& _block)
{
	pair<string, string> block;
	for (auto &statement: _block.statements())
	{
		block.first += visit(statement).first;
		block.second += visit(statement).second;
	}
	return block;
}

pair<string, string> ProtoConverter::visit(Statement const& _stmt)
{
	switch (_stmt.stmt_oneof_case())
	{
	case Statement::kVar:
		return visit(_stmt.var(), false);
	case Statement::kIfstmt:
		return visit(_stmt.ifstmt());
	case Statement::kForstmt:
		return make_pair("", visit(_stmt.forstmt()));
	case Statement::kSwitchstmt:
		return make_pair("", visit(_stmt.switchstmt()));
	case Statement::kBreakstmt:
		return make_pair("", visit(_stmt.breakstmt()));
	case Statement::kContinuestmt:
		return make_pair("", visit(_stmt.continuestmt()));
	case Statement::kReturnstmt:
		return make_pair("", visit(_stmt.returnstmt()));
	case Statement::kDostmt:
		return visit(_stmt.dostmt());
	case Statement::kWhilestmt:
		return visit(_stmt.whilestmt());
	case Statement::STMT_ONEOF_NOT_SET:
		return make_pair("", "");
	}
}

pair<string, string> ProtoConverter::visit(solidity::test::abiv2fuzzer::VarDecl const& _varDecl, bool _stateVar)
{
	solidity::test::abiv2fuzzer::ProtoConverter converter;
	converter.m_isStateVar = _stateVar;
	converter.m_varCounter = m_numVars;
	converter.m_structCounter = m_numStructs;
	auto decl = converter.visit(_varDecl);
	if (!decl.first.empty())
	{
		m_numVars++;
		m_numStructs += converter.m_numStructsAdded;
		decl.second += "\n" +
			solidity::test::abiv2fuzzer::AssignCheckVisitor{
				solidity::test::abiv2fuzzer::ProtoConverter::s_stateVarNamePrefix +
				to_string(m_numVars - 1),
				"",
				0,
				"true",
				0,
				m_numStructs - 1
			}.visit(_varDecl.type()).second;
	}
	return decl;
}

pair<string, string> ProtoConverter::visit(IfStmt const& _ifstmt)
{
	string ifCond = visit(_ifstmt.condition());
	pair<string, string> buffer = visit(_ifstmt.statements());

	return make_pair(buffer.first, Whiskers(R"(if (<cond>) {
	<statements>
	}
	)")
	("cond", ifCond)
	("statements", buffer.second)
	.render());
}

string ProtoConverter::visit(ForStmt const&)
{
	return "";
}

string ProtoConverter::visit(SwitchStmt const&)
{
	return "";
}

string ProtoConverter::visit(BreakStmt const&)
{
	return "break;\n";
}

string ProtoConverter::visit(ContinueStmt const&)
{
	return "continue;\n";
}

string ProtoConverter::visit(ReturnStmt const& _returnstmt)
{
	return Whiskers(R"(return <expr>;
)")
		("expr", visit(_returnstmt.value()))
		.render();
}

pair<string, string> ProtoConverter::visit(DoStmt const& _dostmt)
{
	string doCond = visit(_dostmt.condition());
	pair<string, string> buffer = visit(_dostmt.statements());

	return make_pair(buffer.first, Whiskers(R"(do {
	<statements>
	} while (<cond>)
	)")
		("cond", doCond)
		("statements", buffer.second)
		.render());
}

pair<string, string> ProtoConverter::visit(WhileStmt const& _whilestmt)
{
	string whileCond = visit(_whilestmt.condition());
	pair<string, string> buffer = visit(_whilestmt.statements());

	return make_pair(buffer.first, Whiskers(R"(while (<cond>) {
	<statements>
	}
	)")
		("cond", whileCond)
		("statements", buffer.second)
		.render());
}

string ProtoConverter::visit(Expression const& _expr)
{
	switch (_expr.expr_oneof_case())
	{
	case Expression::kLit:
		return visit(_expr.lit());
	case Expression::kBop:
		return visit(_expr.bop());
	case Expression::kUop:
		return visit(_expr.uop());
	case Expression::kRef:
		return visit(_expr.ref());
	case Expression::EXPR_ONEOF_NOT_SET:
		return "\"\"";
	}
}

string ProtoConverter::visit(Literal const& _literal)
{
	switch (_literal.literal_oneof_case())
	{
	case Literal::kBlit:
		return _literal.blit() ? "true" : "false";
	case Literal::kSlit:
		return "\"" + _literal.slit() + "\"";
	case Literal::LITERAL_ONEOF_NOT_SET:
		return "\"\"";
	}
}

string ProtoConverter::visit(BinaryOp const& _bop)
{
	string op;
	switch (_bop.op())
	{
	case BinaryOp_Operation_ADD:
		op = "+";
		break;
	case BinaryOp_Operation_SUB:
		op = "-";
		break;
	case BinaryOp_Operation_MUL:
		op = "*";
		break;
	case BinaryOp_Operation_DIV:
		op = "/";
		break;
	}
	return Whiskers(R"(<arg1> <op> <arg2>)")
		("arg1", visit(_bop.arg1()))
		("op", op)
		("arg2", visit(_bop.arg2()))
		.render();
}

string ProtoConverter::visit(UnaryOp const& _uop)
{
	string op;
	switch (_uop.op())
	{
		case UnaryOp_Operation_INC:
			op = "++";
			break;
		case UnaryOp_Operation_DEC:
			op = "--";
			break;
	}
	return Whiskers(R"(<arg><op>)")
		("arg", visit(_uop.arg()))
		("op", op)
		.render();
}

string ProtoConverter::visit(VarRef const& _varref)
{
	if (m_numVars > 0)
		return solidity::test::abiv2fuzzer::ProtoConverter::s_stateVarNamePrefix + to_string(_varref.varnum() % m_numVars);
	else
		return "\"\"";
}

string ProtoConverter::functionVisibility(ContractFunction::Visibility _vis)
{
	switch (_vis)
	{
	case ContractFunction_Visibility_PUBLIC:
		return "public";
	case ContractFunction_Visibility_PRIVATE:
		return "private";
	case ContractFunction_Visibility_EXTERNAL:
		return "external";
	case ContractFunction_Visibility_INTERNAL:
		return "internal";
	}
}

string ProtoConverter::functionVisibility(LibraryFunction::Visibility _vis)
{
	switch (_vis)
	{
	case LibraryFunction_Visibility_PUBLIC:
		return "public";
	case LibraryFunction_Visibility_PRIVATE:
		return "private";
	case LibraryFunction_Visibility_EXTERNAL:
		return "external";
	case LibraryFunction_Visibility_INTERNAL:
		return "internal";
	}
}

string ProtoConverter::stateMutability(ContractFunction::StateMutability _mut)
{
	switch (_mut)
	{
	case ContractFunction_StateMutability_PURE:
		return "pure";
	case ContractFunction_StateMutability_VIEW:
		return "view";
	case ContractFunction_StateMutability_PAYABLE:
		return "payable";
	}
}

string ProtoConverter::stateMutability(LibraryFunction::StateMutability _mut)
{
	switch (_mut)
	{
	case LibraryFunction_StateMutability_PURE:
		return "pure";
	case LibraryFunction_StateMutability_VIEW:
		return "view";
	}
}

string ProtoConverter::stateMutability(InterfaceFunction::StateMutability _mut)
{
	switch (_mut)
	{
	case InterfaceFunction_StateMutability_PURE:
		return "pure";
	case InterfaceFunction_StateMutability_VIEW:
		return "view";
	case InterfaceFunction_StateMutability_PAYABLE:
		return "payable";
	}
}