#include <test/tools/ossfuzz/abiV2FuzzerCommon.h>

using namespace solidity::test::abiv2fuzzer;

SolidityCompilationFramework::SolidityCompilationFramework(langutil::EVMVersion _evmVersion)
{
	m_evmVersion = _evmVersion;
}

solidity::bytes SolidityCompilationFramework::compileContract(
	std::string const& _sourceCode,
	std::string const& _contractName,
	std::map<std::string, solidity::util::h160> const& _libraryAddresses,
	frontend::OptimiserSettings _optimization
)
{
	std::string sourceCode = _sourceCode;
	m_compiler.setSources({{"", sourceCode}});
	m_compiler.setLibraries(_libraryAddresses);
	m_compiler.setEVMVersion(m_evmVersion);
	m_compiler.setOptimiserSettings(_optimization);
	if (!m_compiler.compile())
	{
		langutil::SourceReferenceFormatter formatter(std::cerr);

		for (auto const& error: m_compiler.errors())
			formatter.printExceptionInformation(
					*error,
					formatter.formatErrorInformation(*error)
			);
		std::cerr << "Compiling contract failed" << std::endl;
	}
	evmasm::LinkerObject obj = m_compiler.object(
		_contractName.empty() ?
		m_compiler.lastContractName() :
		_contractName
	);
	return obj.bytecode;
}
