#pragma once

#include <libsolidity/interface/CompilerStack.h>

#include <libyul/AssemblyStack.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/Keccak256.h>

namespace solidity::test::abiv2fuzzer
{

class SolidityCompilationFramework
{
public:
	explicit SolidityCompilationFramework(langutil::EVMVersion _evmVersion = {});

	Json::Value getMethodIdentifiers()
	{
		return m_compiler.methodIdentifiers(m_compiler.lastContractName());
	}
	bytes compileContract(
		std::string const& _sourceCode,
		std::string const& _contractName,
		std::map<std::string, solidity::util::h160> const& _libraryAddresses = {},
		frontend::OptimiserSettings _optimization = frontend::OptimiserSettings::minimal()
	);
protected:
	frontend::CompilerStack m_compiler;
	langutil::EVMVersion m_evmVersion;
};

}
