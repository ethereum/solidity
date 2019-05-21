#include <test/tools/ossfuzz/abiV2FuzzerCommon.h>

using namespace dev::test::abiv2fuzzer;

SolidityCompilationFramework::SolidityCompilationFramework(langutil::EVMVersion _evmVersion) : m_evmVersion(_evmVersion) {}

dev::bytes SolidityCompilationFramework::compileContract(std::string const& _sourceCode)
{
	std::string sourceCode = _sourceCode;
	m_compiler.setSources({{"", sourceCode}});
	m_compiler.setEVMVersion(m_evmVersion);
	m_compiler.setOptimiserSettings(m_optimiserSettings);
	if (!m_compiler.compile())
	{
		langutil::SourceReferenceFormatter formatter(std::cerr);

		for (auto const& error: m_compiler.errors())
			formatter.printExceptionInformation(
					*error,
					formatter.formatErrorInformation(*error)
			);
		solAssert(false, "Compiling contract failed");
	}
	dev::eth::LinkerObject obj = m_compiler.runtimeObject(m_compiler.lastContractName());
	return obj.bytecode;
}