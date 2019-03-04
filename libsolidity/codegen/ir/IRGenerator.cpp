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
 * @author Alex Beregszaszi
 * @date 2017
 * Component that translates Solidity code into Yul.
 */

#include <libsolidity/codegen/ir/IRGenerator.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>

#include <libyul/AssemblyStack.h>

#include <libdevcore/CommonData.h>
#include <libdevcore/Whiskers.h>
#include <libdevcore/StringUtils.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

pair<string, string> IRGenerator::run(ContractDefinition const& _contract)
{
	// TODO Would be nice to pretty-print this while retaining comments.
	string ir = generateIR(_contract);

	yul::AssemblyStack asmStack(m_evmVersion, yul::AssemblyStack::Language::StrictAssembly, m_optimiserSettings);
	if (!asmStack.parseAndAnalyze("", ir))
	{
		string errorMessage;
		for (auto const& error: asmStack.errors())
			errorMessage += langutil::SourceReferenceFormatter::formatExceptionInformation(
				*error,
				(error->type() == langutil::Error::Type::Warning) ? "Warning" : "Error"
			);
		solAssert(false, "Invalid IR generated:\n" + errorMessage + "\n" + ir);
	}
	asmStack.optimize();

	string warning =
		"/*******************************************************\n"
		" *                       WARNING                       *\n"
		" *  Solidity to Yul compilation is still EXPERIMENTAL  *\n"
		" *       It can result in LOSS OF FUNDS or worse       *\n"
		" *                !USE AT YOUR OWN RISK!               *\n"
		" *******************************************************/\n\n";

	return {warning + ir, warning + asmStack.print()};
}

string IRGenerator::generateIR(ContractDefinition const& _contract)
{
	Whiskers t(R"(
		object "<CreationObject>" {
			code {
				<memoryInit>
				<constructor>
				<deploy>
				<functions>
			}
			object "<RuntimeObject>" {
				code {
					<memoryInit>
					<dispatch>
					<runtimeFunctions>
				}
			}
		}
	)");

	resetContext();
	t("CreationObject", creationObjectName(_contract));
	t("memoryInit", memoryInit());
	t("constructor", _contract.constructor() ? constructorCode(*_contract.constructor()) : "");
	t("deploy", deployCode(_contract));
	t("functions", m_context.functionCollector()->requestedFunctions());

	resetContext();
	t("RuntimeObject", runtimeObjectName(_contract));
	t("dispatch", dispatchRoutine(_contract));
	t("runtimeFunctions", m_context.functionCollector()->requestedFunctions());
	return t.render();
}

string IRGenerator::generateIRFunction(FunctionDefinition const& _function)
{
	string functionName = "fun_" + to_string(_function.id()) + "_" + _function.name();
	return m_context.functionCollector()->createFunction(functionName, [&]() {
		Whiskers t("function <functionName>(<params>) <returns> {}");
		t("functionName", functionName);
		string params;
		for (auto const& varDecl: _function.parameters())
			params += (params.empty() ? "" : ", ") + m_context.addLocalVariable(*varDecl);
		t("params", params);
		string retParams;
		for (auto const& varDecl: _function.returnParameters())
			retParams += (retParams.empty() ? "" : ", ") + m_context.addLocalVariable(*varDecl);
		t("returns", retParams.empty() ? "" : " -> " + retParams);
		return t.render();
	});
}

string IRGenerator::constructorCode(FunctionDefinition const& _constructor)
{
	string out;
	if (!_constructor.isPayable())
		out = callValueCheck();

	solUnimplemented("Constructors are not yet implemented.");

	return out;
}

string IRGenerator::deployCode(ContractDefinition const& _contract)
{
	Whiskers t(R"X(
		codecopy(0, dataoffset("<object>"), datasize("<object>"))
		return(0, datasize("<object>"))
	)X");
	t("object", runtimeObjectName(_contract));
	return t.render();
}

string IRGenerator::callValueCheck()
{
	return "if callvalue() { revert(0, 0) }";
}

string IRGenerator::creationObjectName(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + to_string(_contract.id());
}

string IRGenerator::runtimeObjectName(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + to_string(_contract.id()) + "_deployed";
}

string IRGenerator::dispatchRoutine(ContractDefinition const& _contract)
{
	Whiskers t(R"X(
		if iszero(lt(calldatasize(), 4))
		{
			let selector := <shr224>(calldataload(0))
			switch selector
			<#cases>
			case <functionSelector>
			{
				// <functionName>
				<callValueCheck>
				<assignToParams> <abiDecode>(4, calldatasize())
				<assignToRetParams> <function>(<params>)
				let memPos := <allocate>(0)
				let memEnd := <abiEncode>(memPos <comma> <retParams>)
				return(memPos, sub(memEnd, memPos))
			}
			</cases>
			default {}
		}
		<fallback>
	)X");
	t("shr224", m_utils.shiftRightFunction(224));
	vector<map<string, string>> functions;
	for (auto const& function: _contract.interfaceFunctions())
	{
		functions.push_back({});
		map<string, string>& templ = functions.back();
		templ["functionSelector"] = "0x" + function.first.hex();
		FunctionTypePointer const& type = function.second;
		templ["functionName"] = type->externalSignature();
		templ["callValueCheck"] = type->isPayable() ? "" : callValueCheck();

		unsigned paramVars = make_shared<TupleType>(type->parameterTypes())->sizeOnStack();
		unsigned retVars = make_shared<TupleType>(type->returnParameterTypes())->sizeOnStack();
		templ["assignToParams"] = paramVars == 0 ? "" : "let " + m_utils.suffixedVariableNameList("param_", 0, paramVars) + " := ";
		templ["assignToRetParams"] = retVars == 0 ? "" : "let " + m_utils.suffixedVariableNameList("ret_", 0, retVars) + " := ";

		ABIFunctions abiFunctions(m_evmVersion, m_context.functionCollector());
		templ["abiDecode"] = abiFunctions.tupleDecoder(type->parameterTypes());
		templ["params"] = m_utils.suffixedVariableNameList("param_", 0, paramVars);
		templ["retParams"] = m_utils.suffixedVariableNameList("ret_", retVars, 0);
		templ["function"] = generateIRFunction(dynamic_cast<FunctionDefinition const&>(type->declaration()));
		templ["allocate"] = m_utils.allocationFunction();
		templ["abiEncode"] = abiFunctions.tupleEncoder(type->returnParameterTypes(), type->returnParameterTypes(), false);
		templ["comma"] = retVars == 0 ? "" : ", ";
	}
	t("cases", functions);
	if (FunctionDefinition const* fallback = _contract.fallbackFunction())
	{
		string fallbackCode;
		if (!fallback->isPayable())
			fallbackCode += callValueCheck();
		fallbackCode += generateIRFunction(*fallback) + "() stop()";

		t("fallback", fallbackCode);
	}
	else
		t("fallback", "revert(0, 0)");
	return t.render();
}

string IRGenerator::memoryInit()
{
	// This function should be called at the beginning of the EVM call frame
	// and thus can assume all memory to be zero, including the contents of
	// the "zero memory area" (the position CompilerUtils::zeroPointer points to).
	return
		Whiskers{"mstore(<memPtr>, <generalPurposeStart>)"}
		("memPtr", to_string(CompilerUtils::freeMemoryPointer))
		("generalPurposeStart", to_string(CompilerUtils::generalPurposeMemoryStart))
		.render();
}

void IRGenerator::resetContext()
{
	solAssert(
		m_context.functionCollector()->requestedFunctions().empty(),
		"Reset context while it still had functions."
	);
	m_context = IRGenerationContext(m_evmVersion, m_optimiserSettings);
	m_utils = YulUtilFunctions(m_evmVersion, m_context.functionCollector());
}
