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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * JSON interface for the solidity compiler to be used from Javascript.
 */

#include <string>
#include <iostream>
#include <json/json.h>
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libevmcore/Instruction.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/ASTPrinter.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/SourceReferenceFormatter.h>
#include <libsolidity/ASTJsonConverter.h>

using namespace std;
using namespace dev;
using namespace solidity;

string formatError(Exception const& _exception, string const& _name, CompilerStack const& _compiler)
{
	ostringstream errorOutput;
	SourceReferenceFormatter::printExceptionInformation(errorOutput, _exception, _name, _compiler);

	Json::Value output(Json::objectValue);
	output["error"] = errorOutput.str();
	return Json::FastWriter().write(output);
}

string compile(string _input, bool _optimize)
{
	StringMap sources;
	sources[""] = _input;

	Json::Value output(Json::objectValue);
	CompilerStack compiler;
	try
	{
		compiler.compile(_input, _optimize);
	}
	catch (ParserError const& exception)
	{
		return formatError(exception, "Parser error", compiler);
	}
	catch (DeclarationError const& exception)
	{
		return formatError(exception, "Declaration error", compiler);
	}
	catch (TypeError const& exception)
	{
		return formatError(exception, "Type error", compiler);
	}
	catch (CompilerError const& exception)
	{
		return formatError(exception, "Compiler error", compiler);
	}
	catch (InternalCompilerError const& exception)
	{
		return formatError(exception, "Internal compiler error", compiler);
	}
	catch (Exception const& exception)
	{
		output["error"] = "Exception during compilation: " + boost::diagnostic_information(exception);
		return Json::FastWriter().write(output);
	}
	catch (...)
	{
		output["error"] = "Unknown exception during compilation.";
		return Json::FastWriter().write(output);
	}

	output["contracts"] = Json::Value(Json::objectValue);
	for (string const& contractName: compiler.getContractNames())
	{
		Json::Value contractData(Json::objectValue);
		contractData["solidity_interface"] = compiler.getSolidityInterface(contractName);
		contractData["interface"] = compiler.getInterface(contractName);
		contractData["bytecode"] = toHex(compiler.getBytecode(contractName));
		contractData["opcodes"] = eth::disassemble(compiler.getBytecode(contractName));
		ostringstream unused;
		contractData["assembly"] = compiler.streamAssembly(unused, contractName, sources, true);
		output["contracts"][contractName] = contractData;
	}

	output["sources"] = Json::Value(Json::objectValue);
	output["sources"][""] = Json::Value(Json::objectValue);
	output["sources"][""]["AST"] = ASTJsonConverter(compiler.getAST("")).json();

	return Json::FastWriter().write(output);
}

static string outputBuffer;

extern "C"
{
extern char const* compileJSON(char const* _input, bool _optimize)
{
	outputBuffer = compile(_input, _optimize);
	return outputBuffer.c_str();
}
}
