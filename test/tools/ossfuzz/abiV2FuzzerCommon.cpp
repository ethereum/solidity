#include <test/tools/ossfuzz/abiV2FuzzerCommon.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

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
		langutil::SourceReferenceFormatter formatter(std::cerr, false, false);

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

bool AbiV2Utility::isOutputExpected(
	uint8_t const* _result,
	size_t _length,
	std::vector<uint8_t> const& _expectedOutput
)
{
	if (_length != _expectedOutput.size())
		return false;

	return (memcmp(_result, _expectedOutput.data(), _length) == 0);
}

evmc_message AbiV2Utility::initializeMessage(bytes const& _input)
{
	// Zero initialize all message fields
	evmc_message msg = {};
	// Gas available (value of type int64_t) is set to its maximum
	// value.
	msg.gas = std::numeric_limits<int64_t>::max();
	msg.input_data = _input.data();
	msg.input_size = _input.size();
	return msg;
}

evmc::result AbiV2Utility::executeContract(
	EVMHost& _hostContext,
	bytes const& _functionHash,
	evmc_address _deployedAddress
)
{
	evmc_message message = initializeMessage(_functionHash);
	message.destination = _deployedAddress;
	message.kind = EVMC_CALL;
	return _hostContext.call(message);
}

evmc::result AbiV2Utility::deployContract(EVMHost& _hostContext, bytes const& _code)
{
	evmc_message message = initializeMessage(_code);
	message.kind = EVMC_CREATE;
	return _hostContext.call(message);
}
