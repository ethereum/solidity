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

#include <test/libsolidity/SemanticTest.h>

#include <libsolutil/Whiskers.h>
#include <libyul/Exceptions.h>
#include <test/Common.h>
#include <test/libsolidity/util/BytesUtils.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/throw_exception.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::frontend::test;
using namespace boost::algorithm;
using namespace boost::unit_test;
using namespace std::string_literals;
namespace fs = boost::filesystem;

std::ostream& solidity::frontend::test::operator<<(std::ostream& _output, RequiresYulOptimizer _requiresYulOptimizer)
{
	switch (_requiresYulOptimizer)
	{
	case RequiresYulOptimizer::False: _output << "false"; break;
	case RequiresYulOptimizer::MinimalStack: _output << "minimalStack"; break;
	case RequiresYulOptimizer::Full: _output << "full"; break;
	}
	return _output;
}

SemanticTest::SemanticTest(
	std::string const& _filename,
	langutil::EVMVersion _evmVersion,
	std::optional<uint8_t> _eofVersion,
	std::vector<boost::filesystem::path> const& _vmPaths,
	bool _enforceGasCost,
	u256 _enforceGasCostMinValue
):
	SolidityExecutionFramework(_evmVersion, _eofVersion, _vmPaths, false),
	EVMVersionRestrictedTestCase(_filename),
	m_sources(m_reader.sources()),
	m_lineOffset(m_reader.lineNumber()),
	m_builtins(makeBuiltins()),
	m_sideEffectHooks(makeSideEffectHooks()),
	m_enforceGasCost(_enforceGasCost),
	m_enforceGasCostMinValue(std::move(_enforceGasCostMinValue))
{
	static std::set<std::string> const compileViaYulAllowedValues{"also", "true", "false"};
	static std::set<std::string> const yulRunTriggers{"also", "true"};
	static std::set<std::string> const legacyRunTriggers{"also", "false", "default"};

	m_requiresYulOptimizer = m_reader.enumSetting<RequiresYulOptimizer>(
		"requiresYulOptimizer",
		{
			{toString(RequiresYulOptimizer::False), RequiresYulOptimizer::False},
			{toString(RequiresYulOptimizer::MinimalStack), RequiresYulOptimizer::MinimalStack},
			{toString(RequiresYulOptimizer::Full), RequiresYulOptimizer::Full},
		},
		toString(RequiresYulOptimizer::False)
	);

	m_runWithABIEncoderV1Only = m_reader.boolSetting("ABIEncoderV1Only", false);
	if (m_runWithABIEncoderV1Only && !solidity::test::CommonOptions::get().useABIEncoderV1)
		m_shouldRun = false;

	std::string compileViaYul = m_reader.stringSetting("compileViaYul", "also");
	if (m_runWithABIEncoderV1Only && compileViaYul != "false")
		BOOST_THROW_EXCEPTION(std::runtime_error(
			"ABIEncoderV1Only tests cannot be run via yul, "
			"so they need to also specify ``compileViaYul: false``"
		));
	if (!util::contains(compileViaYulAllowedValues, compileViaYul))
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid compileViaYul value: " + compileViaYul + "."));
	m_testCaseWantsYulRun = util::contains(yulRunTriggers, compileViaYul);
	m_testCaseWantsLegacyRun = util::contains(legacyRunTriggers, compileViaYul);

	auto revertStrings = revertStringsFromString(m_reader.stringSetting("revertStrings", "default"));
	soltestAssert(revertStrings, "Invalid revertStrings setting.");
	m_revertStrings = revertStrings.value();

	m_allowNonExistingFunctions = m_reader.boolSetting("allowNonExistingFunctions", false);

	parseExpectations(m_reader.stream());
	soltestAssert(!m_tests.empty(), "No tests specified in " + _filename);

	if (m_enforceGasCost)
	{
		m_compiler.setMetadataFormat(CompilerStack::MetadataFormat::NoMetadata);
		m_compiler.setMetadataHash(CompilerStack::MetadataHash::None);
	}
}

std::map<std::string, Builtin> SemanticTest::makeBuiltins()
{
	return {
		{
			"isoltest_builtin_test",
			[](FunctionCall const&) -> std::optional<bytes>
			{
				return toBigEndian(u256(0x1234));
			}
		},
		{
			"isoltest_side_effects_test",
			[](FunctionCall const& _call) -> std::optional<bytes>
			{
				if (_call.arguments.parameters.empty())
					return toBigEndian(0);
				else
					return _call.arguments.rawBytes();
			}
		},
		{
			"balance",
			[this](FunctionCall const& _call) -> std::optional<bytes>
			{
				soltestAssert(_call.arguments.parameters.size() <= 1, "Account address expected.");
				h160 address;
				if (_call.arguments.parameters.size() == 1)
					address = h160(_call.arguments.parameters.at(0).rawString);
				else
					address = m_contractAddress;
				return toBigEndian(balanceAt(address));
			}
		},
		{
			"storageEmpty",
			[this](FunctionCall const& _call) -> std::optional<bytes>
			{
				soltestAssert(_call.arguments.parameters.empty(), "No arguments expected.");
				return toBigEndian(u256(storageEmpty(m_contractAddress) ? 1 : 0));
		 	}
		},
		{
			"account",
			[this](FunctionCall const& _call) -> std::optional<bytes>
			{
				soltestAssert(_call.arguments.parameters.size() == 1, "Account number expected.");
				size_t accountNumber = static_cast<size_t>(stoi(_call.arguments.parameters.at(0).rawString));
				// Need to pad it to 32-bytes to workaround limitations in BytesUtils::formatHex.
				return toBigEndian(h256(ExecutionFramework::setAccount(accountNumber).asBytes(), h256::AlignRight));
			}
		},
	};
}

std::vector<SideEffectHook> SemanticTest::makeSideEffectHooks() const
{
	using namespace std::placeholders;
	return {
		[](FunctionCall const& _call) -> std::vector<std::string>
		{
			if (_call.signature == "isoltest_side_effects_test")
			{
				std::vector<std::string> result;
				for (auto const& argument: _call.arguments.parameters)
					result.emplace_back(util::toHex(argument.rawBytes));
				return result;
			}
			return {};
		},
		bind(&SemanticTest::eventSideEffectHook, this, _1)
	};
}

std::string SemanticTest::formatEventParameter(std::optional<AnnotatedEventSignature> _signature, bool _indexed, size_t _index, bytes const& _data)
{
	auto isPrintableASCII = [](bytes const& s)
	{
		bool zeroes = true;
		for (auto c: s)
		{
			if (static_cast<unsigned>(c) != 0x00)
			{
				zeroes = false;
				if (static_cast<unsigned>(c) <= 0x1f || static_cast<unsigned>(c) >= 0x7f)
					return false;
			} else
				break;
		}
		return !zeroes;
	};

	ABIType abiType(ABIType::Type::Hex);
	if (isPrintableASCII(_data))
		abiType = ABIType(ABIType::Type::String);
	if (_signature.has_value())
	{
		std::vector<std::string> const& types = _indexed ? _signature->indexedTypes : _signature->nonIndexedTypes;
		if (_index < types.size())
		{
			if (types.at(_index) == "bool")
				abiType = ABIType(ABIType::Type::Boolean);
		}
	}
	return BytesUtils::formatBytes(_data, abiType);
}

std::vector<std::string> SemanticTest::eventSideEffectHook(FunctionCall const&) const
{
	std::vector<std::string> sideEffects;
	std::vector<LogRecord> recordedLogs = ExecutionFramework::recordedLogs();
	for (LogRecord const& log: recordedLogs)
	{
		std::optional<AnnotatedEventSignature> eventSignature;
		if (!log.topics.empty())
			eventSignature = matchEvent(log.topics[0]);
		std::stringstream sideEffect;
		sideEffect << "emit ";
		if (eventSignature.has_value())
			sideEffect << eventSignature.value().signature;
		else
			sideEffect << "<anonymous>";

		if (m_contractAddress != log.creator)
			sideEffect << " from 0x" << log.creator;

		std::vector<std::string> eventStrings;
		size_t index{0};
		for (h256 const& topic: log.topics)
		{
			if (!eventSignature.has_value() || index != 0)
				eventStrings.push_back("#" + formatEventParameter(eventSignature, true, index, topic.asBytes()));
			++index;
		}

		soltestAssert(log.data.size() % 32 == 0, "");
		for (size_t index = 0; index < log.data.size() / 32; ++index)
		{
			auto begin = log.data.begin() + static_cast<long>(index * 32);
			bytes const& data = bytes{begin, begin + 32};
			eventStrings.emplace_back(formatEventParameter(eventSignature, false, index, data));
		}

		if (!eventStrings.empty())
			sideEffect << ": ";
		sideEffect << joinHumanReadable(eventStrings);
		sideEffects.emplace_back(sideEffect.str());
	}
	return sideEffects;
}

std::optional<AnnotatedEventSignature> SemanticTest::matchEvent(util::h256 const& hash) const
{
	std::optional<AnnotatedEventSignature> result;
	for (std::string& contractName: m_compiler.contractNames())
	{
		ContractDefinition const& contract = m_compiler.contractDefinition(contractName);
		for (EventDefinition const* event: contract.events() + contract.usedInterfaceEvents())
		{
			FunctionTypePointer eventFunctionType = event->functionType(true);
			if (!event->isAnonymous() && keccak256(eventFunctionType->externalSignature()) == hash)
			{
				AnnotatedEventSignature eventInfo;
				eventInfo.signature = eventFunctionType->externalSignature();
				for (auto const& param: event->parameters())
					if (param->isIndexed())
						eventInfo.indexedTypes.emplace_back(param->type()->toString(true));
					else
						eventInfo.nonIndexedTypes.emplace_back(param->type()->toString(true));
				result = eventInfo;
			}
		}
	}
	return result;
}

frontend::OptimiserSettings SemanticTest::optimizerSettingsFor(RequiresYulOptimizer _requiresYulOptimizer)
{
	switch (_requiresYulOptimizer)
	{
	case RequiresYulOptimizer::False:
		return OptimiserSettings::minimal();
	case RequiresYulOptimizer::MinimalStack:
	{
		OptimiserSettings settings = OptimiserSettings::minimal();
		settings.runYulOptimiser = true;
		settings.yulOptimiserSteps = "uljmul jmul";
		return settings;
	}
	case RequiresYulOptimizer::Full:
		return OptimiserSettings::full();
	}
	unreachable();
}

TestCase::TestResult SemanticTest::run(std::ostream& _stream, std::string const& _linePrefix, bool _formatted)
{
	TestResult result = TestResult::Success;

	if (m_testCaseWantsLegacyRun && !m_eofVersion.has_value())
		result = runTest(_stream, _linePrefix, _formatted, false /* _isYulRun */);

	if (m_testCaseWantsYulRun && result == TestResult::Success)
	{
		if (solidity::test::CommonOptions::get().optimize)
			result = runTest(_stream, _linePrefix, _formatted, true /* _isYulRun */);
		else
			result = tryRunTestWithYulOptimizer(_stream, _linePrefix, _formatted);
	}

	if (result != TestResult::Success)
		solidity::test::CommonOptions::get().printSelectedOptions(
			_stream,
			_linePrefix,
			{"evmVersion", "optimize", "useABIEncoderV1", "batch"}
		);

	return result;
}

TestCase::TestResult SemanticTest::runTest(
	std::ostream& _stream,
	std::string const& _linePrefix,
	bool _formatted,
	bool _isYulRun
)
{
	bool success = true;
	m_gasCostFailure = false;

	selectVM(evmc_capabilities::EVMC_CAPABILITY_EVM1);

	reset();

	m_compileViaYul = _isYulRun;

	if (_isYulRun)
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Running via Yul: " << std::endl;

	for (TestFunctionCall& test: m_tests)
		test.reset();

	std::map<std::string, solidity::test::Address> libraries;

	bool constructed = false;

	for (TestFunctionCall& test: m_tests)
	{
		if (constructed)
		{
			soltestAssert(
				test.call().kind != FunctionCall::Kind::Library,
				"Libraries have to be deployed before any other call."
			);
			soltestAssert(
				test.call().kind != FunctionCall::Kind::Constructor,
				"Constructor has to be the first function call expect for library deployments."
			);
		}
		else if (test.call().kind == FunctionCall::Kind::Library)
		{
			soltestAssert(
				deploy(test.call().signature, 0, {}, libraries) && m_transactionSuccessful,
				"Failed to deploy library " + test.call().signature);
			// For convenience, in semantic tests we assume that an unqualified name like `L` is equivalent to one
			// with an empty source unit name (`:L`). This is fine because the compiler never uses unqualified
			// names in the Yul code it produces and does not allow `linkersymbol()` at all in inline assembly.
			libraries[test.call().libraryFile + ":" + test.call().signature] = m_contractAddress;
			continue;
		}
		else
		{
			if (test.call().kind == FunctionCall::Kind::Constructor)
				deploy("", test.call().value.value, test.call().arguments.rawBytes(), libraries);
			else
				soltestAssert(deploy("", 0, bytes(), libraries), "Failed to deploy contract.");
			constructed = true;
		}

		if (test.call().kind == FunctionCall::Kind::Constructor)
		{
			if (m_transactionSuccessful == test.call().expectations.failure)
				success = false;
			if (success && !checkGasCostExpectation(test, _isYulRun))
			{
				success = false;
				m_gasCostFailure = true;
			}

			test.setFailure(!m_transactionSuccessful);
			test.setRawBytes(bytes());
		}
		else
		{
			bytes output;
			if (test.call().kind == FunctionCall::Kind::LowLevel)
				output = callLowLevel(test.call().arguments.rawBytes(), test.call().value.value);
			else if (test.call().kind == FunctionCall::Kind::Builtin)
			{
				std::optional<bytes> builtinOutput = m_builtins.at(test.call().signature)(test.call());
				if (builtinOutput.has_value())
				{
					m_transactionSuccessful = true;
					output = builtinOutput.value();
				}
				else
					m_transactionSuccessful = false;
			}
			else
			{
				soltestAssert(
					m_allowNonExistingFunctions ||
					m_compiler.interfaceSymbols(m_compiler.lastContractName(m_sources.mainSourceFile))["methods"].isMember(test.call().signature),
					"The function " + test.call().signature + " is not known to the compiler"
				);

				output = callContractFunctionWithValueNoEncoding(
					test.call().signature,
					test.call().value.value,
					test.call().arguments.rawBytes()
				);
			}

			bool outputMismatch = (output != test.call().expectations.rawBytes());
			if (!outputMismatch && !checkGasCostExpectation(test, _isYulRun))
			{
				success = false;
				m_gasCostFailure = true;
			}

			// Pre byzantium, it was not possible to return failure data, so we disregard
			// output mismatch for those EVM versions.
			if (test.call().expectations.failure && !m_transactionSuccessful && !m_evmVersion.supportsReturndata())
				outputMismatch = false;
			if (m_transactionSuccessful != !test.call().expectations.failure || outputMismatch)
				success = false;

			test.setFailure(!m_transactionSuccessful);
			test.setRawBytes(std::move(output));
			if (test.call().kind != FunctionCall::Kind::LowLevel)
				test.setContractABI(m_compiler.contractABI(m_compiler.lastContractName(m_sources.mainSourceFile)));
		}

		std::vector<std::string> effects;
		for (SideEffectHook const& hook: m_sideEffectHooks)
			effects += hook(test.call());
		test.setSideEffects(std::move(effects));

		success &= test.call().expectedSideEffects == test.call().actualSideEffects;
	}

	if (!success)
	{
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << std::endl;
		for (TestFunctionCall const& test: m_tests)
		{
			ErrorReporter errorReporter;
			_stream << test.format(
				errorReporter,
				_linePrefix,
				TestFunctionCall::RenderMode::ExpectedValuesExpectedGas,
				_formatted,
				/* _interactivePrint */ true
			) << std::endl;
			_stream << errorReporter.format(_linePrefix, _formatted);
		}
		_stream << std::endl;
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << std::endl;
		for (TestFunctionCall const& test: m_tests)
		{
			ErrorReporter errorReporter;
			_stream << test.format(
				errorReporter,
				_linePrefix,
				m_gasCostFailure ? TestFunctionCall::RenderMode::ExpectedValuesActualGas : TestFunctionCall::RenderMode::ActualValuesExpectedGas,
				_formatted,
				/* _interactivePrint */ true
			) << std::endl;
			_stream << errorReporter.format(_linePrefix, _formatted);
		}
		AnsiColorized(_stream, _formatted, {BOLD, RED})
			<< _linePrefix << std::endl
			<< _linePrefix << "Attention: Updates on the test will apply the detected format displayed." << std::endl;
		if (_isYulRun && m_testCaseWantsLegacyRun)
		{
			_stream << _linePrefix << std::endl << _linePrefix;
			AnsiColorized(_stream, _formatted, {RED_BACKGROUND}) << "Note that the test passed without Yul.";
			_stream << std::endl;
		}
		else if (!_isYulRun && m_testCaseWantsYulRun)
			AnsiColorized(_stream, _formatted, {BOLD, YELLOW})
				<< _linePrefix << std::endl
				<< _linePrefix << "Note that the test also has to pass via Yul." << std::endl;
		return TestResult::Failure;
	}

	return TestResult::Success;
}

TestCase::TestResult SemanticTest::tryRunTestWithYulOptimizer(
	std::ostream& _stream,
	std::string const& _linePrefix,
	bool _formatted
)
{
	TestResult result{};
	for (auto requiresYulOptimizer: {
		RequiresYulOptimizer::False,
		RequiresYulOptimizer::MinimalStack,
		RequiresYulOptimizer::Full,
	})
	{
		ScopedSaveAndRestore optimizerSettings(
			m_optimiserSettings,
			optimizerSettingsFor(requiresYulOptimizer)
		);

		try
		{
			result = runTest(_stream, _linePrefix, _formatted, true /* _isYulRun */);
		}
		catch (yul::StackTooDeepError const&)
		{
			if (requiresYulOptimizer == RequiresYulOptimizer::Full)
				throw;
			else
				continue;
		}

		if (m_requiresYulOptimizer != requiresYulOptimizer && result != TestResult::FatalError)
		{
			soltestAssert(result == TestResult::Success || result == TestResult::Failure);

			AnsiColorized(_stream, _formatted, {BOLD, YELLOW})
				<< _linePrefix << std::endl
				<< _linePrefix << "requiresYulOptimizer is set to " << m_requiresYulOptimizer
				<< " but should be " << requiresYulOptimizer << std::endl;
			m_requiresYulOptimizer = requiresYulOptimizer;
			return TestResult::Failure;
		}

		return result;
	}
	unreachable();
}

bool SemanticTest::checkGasCostExpectation(TestFunctionCall& io_test, bool _compileViaYul) const
{
	std::string setting =
		(_compileViaYul ? "ir"s : "legacy"s) +
		(m_optimiserSettings == OptimiserSettings::full() ? "Optimized" : "");

	// We don't check gas if enforce gas cost is not active
	// or test is run with abi encoder v1 only
	// or gas used less than threshold for enforcing feature
	// or the test has used up all available gas (test will fail anyway)
	// or setting is "ir" and it's not included in expectations
	// or if the called function is an isoltest builtin e.g. `smokeTest` or `storageEmpty`
	if (
		!m_enforceGasCost ||
		m_gasUsed < m_enforceGasCostMinValue ||
		m_gasUsed >= InitialGas ||
		(setting == "ir" && io_test.call().expectations.gasUsed.count(setting) == 0) ||
		io_test.call().kind == FunctionCall::Kind::Builtin
	)
		return true;

	solAssert(!m_runWithABIEncoderV1Only, "");

	io_test.setGasCost(setting, m_gasUsed);
	return
		io_test.call().expectations.gasUsed.count(setting) > 0 &&
		m_gasUsed == io_test.call().expectations.gasUsed.at(setting);
}

void SemanticTest::printSource(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const
{
	if (m_sources.sources.empty())
		return;

	bool outputNames = (m_sources.sources.size() - m_sources.externalSources.size() != 1 || !m_sources.sources.begin()->first.empty());

	std::set<std::string> externals;
	for (auto const& [name, path]: m_sources.externalSources)
	{
		externals.insert(name);
		std::string externalSource;
		if (name == path)
			externalSource = name;
		else
			externalSource = name + "=" + path.generic_string();

		if (_formatted)
			_stream << _linePrefix  << formatting::CYAN << "==== ExternalSource: " << externalSource << " ===="s << formatting::RESET << std::endl;
		else
			_stream << _linePrefix << "==== ExternalSource: " << externalSource << " ===="s << std::endl;
	}

	for (auto const& [name, source]: m_sources.sources)
		if (externals.find(name) == externals.end())
		{
			if (_formatted)
			{
				if (source.empty())
					continue;

				if (outputNames)
					_stream << _linePrefix << formatting::CYAN << "==== Source: " << name
							<< " ====" << formatting::RESET << std::endl;

				std::vector<char const*> sourceFormatting(source.length(), formatting::RESET);
				_stream << _linePrefix << sourceFormatting.front() << source.front();
				for (size_t i = 1; i < source.length(); i++)
				{
					if (sourceFormatting[i] != sourceFormatting[i - 1])
						_stream << sourceFormatting[i];
					if (source[i] != '\n')
						_stream << source[i];
					else
					{
						_stream << formatting::RESET << std::endl;
						if (i + 1 < source.length())
							_stream << _linePrefix << sourceFormatting[i];
					}
				}
				_stream << formatting::RESET;
			}
			else
			{
				if (outputNames)
					_stream << _linePrefix << "==== Source: " + name << " ====" << std::endl;
				std::stringstream stream(source);
				std::string line;
				while (getline(stream, line))
					_stream << _linePrefix << line << std::endl;
			}
		}
}

void SemanticTest::printUpdatedExpectations(std::ostream& _stream, std::string const&) const
{
	for (TestFunctionCall const& test: m_tests)
		_stream << test.format(
			"",
			m_gasCostFailure ? TestFunctionCall::RenderMode::ExpectedValuesActualGas : TestFunctionCall::RenderMode::ActualValuesExpectedGas,
			/* _highlight = */ false
		) << std::endl;
}

void SemanticTest::printUpdatedSettings(std::ostream& _stream, std::string const& _linePrefix)
{
	auto& settings = m_reader.settings();
	if (settings.empty() && m_requiresYulOptimizer == RequiresYulOptimizer::False)
		return;

	_stream << _linePrefix << "// ====" << std::endl;
	if (m_requiresYulOptimizer != RequiresYulOptimizer::False)
		_stream << _linePrefix << "// requiresYulOptimizer: " << m_requiresYulOptimizer << std::endl;

	for (auto const& [settingName, settingValue]: settings)
		if (settingName != "requiresYulOptimizer")
			_stream << _linePrefix << "// " << settingName << ": " << settingValue<< std::endl;
}

void SemanticTest::parseExpectations(std::istream& _stream)
{
	m_tests += TestFileParser{_stream, m_builtins}.parseFunctionCalls(m_lineOffset);
}

bool SemanticTest::deploy(
	std::string const& _contractName,
	u256 const& _value,
	bytes const& _arguments,
	std::map<std::string, solidity::test::Address> const& _libraries
)
{
	auto output = compileAndRunWithoutCheck(m_sources.sources, _value, _contractName, _arguments, _libraries, m_sources.mainSourceFile);
	return !output.empty() && m_transactionSuccessful;
}
