#include <test/tools/ossfuzz/solarithprotoToSol.h>
#include <test/tools/ossfuzz/protoToYul.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/Whiskers.h>

#include <libyul/AssemblyStack.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/Exceptions.h>

using namespace solidity::test::solarithfuzzer;
using namespace solidity;
using namespace solidity::util;
using namespace std;

string ProtoConverter::programToString(Program const& _program)
{
	m_rand = make_unique<SolRandomNumGenerator>(
		SolRandomNumGenerator(_program.seed())
	);
	return visit(_program);
}

string ProtoConverter::visit(Program const& _program)
{
	Whiskers p(R"(pragma solidity >= 0.0.0;)");
	Whiskers c(R"(contract C {)");
	Whiskers t(R"(function test() public returns (uint)<body>)");
	t("body", visit(_program.b()));
	return p.render()
		+ '\n'
		+ c.render()
		+ '\n'
		+ '\t'
		+ t.render()
		+ '\n'
		+ '}';
}

string ProtoConverter::visit(Block const& _block)
{
	ostringstream blockStr;
	blockStr << '\n'
		<< '\t'
		<< '{'
		<< '\n';
	for (auto const& v: _block.v())
		blockStr << visit(v);
	blockStr << visit(_block.a());
	ostringstream trace;
	if (m_yulAssembly.empty())
		blockStr << "\t\treturn 0;";
	else
		blockStr << addChecks(m_yulProgram, langutil::EVMVersion::berlin(), trace);
	blockStr << '\n'
		<< '\t'
		<< '}';
	return blockStr.str();
}

string ProtoConverter::addChecks(
	string const& _yulSource,
	langutil::EVMVersion _version,
	ostringstream& _os
)
{
	ostringstream out;
	unsigned error = 1;
	auto memoryDump = interpretYul(_yulSource, _version, _os);
	unsigned index = 0;
	for (auto const& v: m_varTypeMap)
	{
		Whiskers check(R"(<ind>if (<var> != <type>(0x<value>)) return <error>;<endl>)");
		check("ind", "\t\t");
		check("var", v.first);
		check("type", get<1>(v.second));
		u256 memIdx = index * 0x20;
		string val{};
		if (memoryDump.count(memIdx))
		{
			unsigned byteWidth = get<2>(v.second);
			val = extractBytes(memoryDump.at(memIdx), byteWidth);
			// Avoid interpretation of 20 byte literals as address literals
			// by prepending 00.
			if (byteWidth == 20)
				val = "00" + val;
		}
		else
			val = "0";
		check("value", val);
		check("error", to_string(error++));
		check("endl", "\n");
		out << check.render();
		index++;
	}
	out << "\t\treturn 0;\n";
	return out.str();
}

map<u256, string> ProtoConverter::interpretYul(
	string const& _yulSource,
	langutil::EVMVersion _version,
	ostringstream& _os
)
{
	using namespace yul;
	using namespace solidity::yul::test::yul_fuzzer;
	YulStringRepository::reset();

	// AssemblyStack entry point
	AssemblyStack stack(
		_version,
		AssemblyStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::full()
	);

	// Parse protobuf mutated YUL code
	if (!stack.parseAndAnalyze("source", _yulSource) || !stack.parserResult()->code ||
	    !stack.parserResult()->analysisInfo)
	{
		std::cout << _yulSource << std::endl;
		yulAssert(false, "Proto fuzzer generated malformed program");
	}
	stack.optimize();
	yulFuzzerUtil i;

	auto r = i.interpret(
		_os,
		stack.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(_version)
	);
	if (r != yulFuzzerUtil::TerminationReason::StepLimitReached && r != yulFuzzerUtil::TerminationReason::TraceLimitReached)
		return i.memoryDump();
	else
		throw langutil::FuzzerError();
}

string ProtoConverter::visit(Assembly const& _as)
{
	using namespace solidity::yul::test;
	ostringstream assemblyStr;
	assemblyStr << "\t\t" << "assembly {\n";
	// Yul converter
	auto c = yul_fuzzer::ProtoConverter{m_varCounter};
	m_yulAssembly = c.programToString(_as.p());
	m_yulProgram = Whiskers("{<endl><init><assembly>}")
		("endl", "\n")
		("init", m_yulInitCode.str())
		("assembly", m_yulAssembly)
		.render();
	assemblyStr << m_yulAssembly;
	assemblyStr << "\t\t}\n";
	return assemblyStr.str();
}

string ProtoConverter::visit(VarDecl const& _vardecl)
{
	Whiskers v(R"(<type> <varName> = <type>(<value>);)");
	string type = visit(_vardecl.t());
	string varName = newVarName();
	unsigned byteWidth = widthUnsigned(_vardecl.t().bytewidth());
	m_varTypeMap.emplace(varName, tuple(typeSign(_vardecl.t()), type, byteWidth));
	v("type", type);
	v("varName", varName);
	string value = maskUnsignedToHex(64);
	v("value", value);
	Whiskers i(R"(let <varName> := <value><endl>)");
	i("varName", varName);
	i("value", "0x" + extractBytes(value, byteWidth));
	i("endl", "\n");
	m_yulInitCode << i.render();
	incrementVarCounter();
	return "\t\t" + v.render() + '\n';
}

string ProtoConverter::visit(Type const& _type)
{
	return signString(_type.s()) + widthString(_type.bytewidth());
}