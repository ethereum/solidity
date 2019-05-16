#pragma once

#include <functional>
#include <libsolidity/interface/CompilerStack.h>
#include <libyul/AssemblyStack.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>
#include <libdevcore/Keccak256.h>

namespace dev
{
namespace test
{
namespace abiv2fuzzer
{
class SolidityExecutionFramework
{
public:
	SolidityExecutionFramework();
	SolidityExecutionFramework(langutil::EVMVersion _evmVersion);

	Json::Value getMethodIdentifiers()
	{
		return m_compiler.methodIdentifiers(m_compiler.lastContractName());
	}

	dev::bytes compileContract(
			std::string const& _sourceCode,
			std::string const& _contractName = "",
			std::map<std::string, dev::FixedHash<20>> const& _libraryAddresses = std::map<std::string, dev::FixedHash<20>>()
	)
	{
		// Silence compiler version warning
		std::string sourceCode = "pragma solidity >=0.0;\n";
		if (_sourceCode.find("pragma experimental ABIEncoderV2;") == std::string::npos)
			sourceCode += "pragma experimental ABIEncoderV2;\n";
		sourceCode += _sourceCode;
		m_compiler.reset();
		m_compiler.setSources({{"", sourceCode}});
		m_compiler.setLibraries(_libraryAddresses);
		m_compiler.setEVMVersion(m_evmVersion);
		m_compiler.setOptimiserSettings(m_optimiserSettings);
		m_compiler.enableIRGeneration(m_compileViaYul);
		if (!m_compiler.compile()) {
			langutil::SourceReferenceFormatter formatter(std::cerr);

			for (auto const& error: m_compiler.errors())
				formatter.printExceptionInformation(
						*error,
						(error->type() == langutil::Error::Type::Warning) ? "Warning" : "Error"
				);
			std::cerr << "Compiling contract failed" << std::endl;
		}
		dev::eth::LinkerObject obj;
		if (m_compileViaYul) {
			yul::AssemblyStack asmStack(
					m_evmVersion,
					yul::AssemblyStack::Language::StrictAssembly,
					m_optimiserSettings
			);
			if (!asmStack.parseAndAnalyze("", m_compiler.yulIROptimized(
					_contractName.empty() ? m_compiler.lastContractName() : _contractName
			))) {
				langutil::SourceReferenceFormatter formatter(std::cerr);

				for (auto const& error: m_compiler.errors())
					formatter.printExceptionInformation(
							*error,
							(error->type() == langutil::Error::Type::Warning) ? "Warning" : "Error"
					);
				std::cerr << "Assembly contract failed. IR: " + m_compiler.yulIROptimized({}) << std::endl;
			}
			asmStack.optimize();
			obj = std::move(*asmStack.assemble(yul::AssemblyStack::Machine::EVM).bytecode);
		} else
			obj = m_compiler.runtimeObject(
					_contractName.empty() ? m_compiler.lastContractName() : _contractName);
		return obj.bytecode;
	}

protected:
	dev::solidity::CompilerStack m_compiler;
	bool m_compileViaYul = false;
	langutil::EVMVersion m_evmVersion;
	dev::solidity::OptimiserSettings m_optimiserSettings = dev::solidity::OptimiserSettings::standard();
};
}
}
}