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
#include <utility>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::frontend::test;
using namespace boost::algorithm;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

SemanticTest::SemanticTest(
	string const& _filename,
	langutil::EVMVersion _evmVersion,
	vector<boost::filesystem::path> const& _vmPaths,
	bool _enforceViaYul,
	bool _enforceCompileToEwasm,
	bool _enforceGasCost,
	u256 _enforceGasCostMinValue
):
	SolidityExecutionFramework(_evmVersion, _vmPaths),
	EVMVersionRestrictedTestCase(_filename),
	m_sources(m_reader.sources()),
	m_lineOffset(m_reader.lineNumber()),
	m_builtins(makeBuiltins()),
	m_sideEffectHooks(makeSideEffectHooks()),
	m_enforceViaYul(_enforceViaYul),
	m_enforceCompileToEwasm(_enforceCompileToEwasm),
	m_enforceGasCost(_enforceGasCost),
	m_enforceGasCostMinValue(move(_enforceGasCostMinValue))
{
	static set<string> const compileViaYulAllowedValues{"also", "true", "false", "default"};
	static set<string> const yulRunTriggers{"also", "true"};
	static set<string> const legacyRunTriggers{"also", "false", "default"};

	string compileViaYul = m_reader.stringSetting("compileViaYul", "default");
	if (!contains(compileViaYulAllowedValues, compileViaYul))
		BOOST_THROW_EXCEPTION(runtime_error("Invalid compileViaYul value: " + compileViaYul + "."));
	m_testCaseWantsYulRun = contains(yulRunTriggers, compileViaYul);
	m_testCaseWantsLegacyRun = contains(legacyRunTriggers, compileViaYul);

	// Do not enforce via yul and ewasm, if via yul was explicitly denied.
	if (compileViaYul == "false")
	{
		m_enforceViaYul = false;
		m_enforceCompileToEwasm = false;
	}

	string compileToEwasm = m_reader.stringSetting("compileToEwasm", "false");
	if (compileToEwasm == "also")
		m_testCaseWantsEwasmRun = true;
	else if (compileToEwasm == "false")
		m_testCaseWantsEwasmRun = false;
	else
		BOOST_THROW_EXCEPTION(runtime_error("Invalid compileToEwasm value: " + compileToEwasm + "."));

	if (m_testCaseWantsEwasmRun && !m_testCaseWantsYulRun)
		BOOST_THROW_EXCEPTION(runtime_error("Invalid compileToEwasm value: " + compileToEwasm + ", compileViaYul need to be enabled."));

	// run ewasm tests only if an ewasm evmc vm was defined
	if (m_testCaseWantsEwasmRun && !m_supportsEwasm)
		m_testCaseWantsEwasmRun = false;

	m_runWithABIEncoderV1Only = m_reader.boolSetting("ABIEncoderV1Only", false);
	if (m_runWithABIEncoderV1Only && !solidity::test::CommonOptions::get().useABIEncoderV1)
		m_shouldRun = false;

	// Sanity check
	if (m_runWithABIEncoderV1Only && (compileViaYul == "true" || compileViaYul == "also"))
		BOOST_THROW_EXCEPTION(runtime_error(
			"ABIEncoderV1Only can not be used with compileViaYul=" + compileViaYul +
			", set it to false or omit the flag. The compileViaYul setting ignores the abicoder pragma"
			" and runs everything with ABICoder V2."
		));

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

map<string, Builtin> SemanticTest::makeBuiltins()
{
	return {
		{
			"isoltest_builtin_test",
			[](FunctionCall const&) -> optional<bytes>
			{
				return toBigEndian(u256(0x1234));
			}
		},
		{
			"isoltest_side_effects_test",
			[](FunctionCall const& _call) -> optional<bytes>
			{
				if (_call.arguments.parameters.empty())
					return toBigEndian(0);
				else
					return _call.arguments.rawBytes();
			}
		},
		{
			"balance",
			[this](FunctionCall const& _call) -> optional<bytes>
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
			[this](FunctionCall const& _call) -> optional<bytes>
			{
				soltestAssert(_call.arguments.parameters.empty(), "No arguments expected.");
				return toBigEndian(u256(storageEmpty(m_contractAddress) ? 1 : 0));
		 	}
		},
		{
			"account",
			[this](FunctionCall const& _call) -> optional<bytes>
			{
				soltestAssert(_call.arguments.parameters.size() == 1, "Account number expected.");
				size_t accountNumber = static_cast<size_t>(stoi(_call.arguments.parameters.at(0).rawString));
				// Need to pad it to 32-bytes to workaround limitations in BytesUtils::formatHex.
				return toBigEndian(h256(ExecutionFramework::setAccount(accountNumber).asBytes(), h256::AlignRight));
			}
		},
	};
}

vector<SideEffectHook> SemanticTest::makeSideEffectHooks() const
{
	using namespace std::placeholders;
	return {
		[](FunctionCall const& _call) -> vector<string>
		{
			if (_call.signature == "isoltest_side_effects_test")
			{
				vector<string> result;
				for (auto const& argument: _call.arguments.parameters)
					result.emplace_back(toHex(argument.rawBytes));
				return result;
			}
			return {};
		},
		bind(&SemanticTest::eventSideEffectHook, this, _1)
	};
}

string SemanticTest::formatEventParameter(optional<AnnotatedEventSignature> _signature, bool _indexed, size_t _index, bytes const& _data)
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
		vector<string> const& types = _indexed ? _signature->indexedTypes : _signature->nonIndexedTypes;
		if (_index < types.size())
		{
			if (types.at(_index) == "bool")
				abiType = ABIType(ABIType::Type::Boolean);
		}
	}
	return BytesUtils::formatBytes(_data, abiType);
}

vector<string> SemanticTest::eventSideEffectHook(FunctionCall const&) const
{
	vector<string> sideEffects;
	vector<LogRecord> recordedLogs = ExecutionFramework::recordedLogs();
	for (LogRecord const& log: recordedLogs)
	{
		optional<AnnotatedEventSignature> eventSignature;
		if (!log.topics.empty())
			eventSignature = matchEvent(log.topics[0]);
		stringstream sideEffect;
		sideEffect << "emit ";
		if (eventSignature.has_value())
			sideEffect << eventSignature.value().signature;
		else
			sideEffect << "<anonymous>";

		if (m_contractAddress != log.creator)
			sideEffect << " from 0x" << log.creator;

		vector<string> eventStrings;
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

optional<AnnotatedEventSignature> SemanticTest::matchEvent(util::h256 const& hash) const
{
	optional<AnnotatedEventSignature> result;
	for (string& contractName: m_compiler.contractNames())
	{
		ContractDefinition const& contract = m_compiler.contractDefinition(contractName);
		for (EventDefinition const* event: contract.events())
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

TestCase::TestResult SemanticTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	TestResult result = TestResult::Success;

	if (m_testCaseWantsLegacyRun)
		result = runTest(_stream, _linePrefix, _formatted, false, false);

	if ((m_testCaseWantsYulRun || m_enforceViaYul) && result == TestResult::Success)
		result = runTest(_stream, _linePrefix, _formatted, true, false);

	if ((m_testCaseWantsEwasmRun || m_enforceCompileToEwasm) && result == TestResult::Success)
	{
		// TODO: Once we have full Ewasm support, we could remove try/catch here.
		try
		{
			result = runTest(_stream, _linePrefix, _formatted, true, true);
		}
		catch (...)
		{
			if (!m_enforceCompileToEwasm)
				throw;
		}
	}
	return result;
}

TestCase::TestResult SemanticTest::runTest(
	ostream& _stream,
	string const& _linePrefix,
	bool _formatted,
	bool _isYulRun,
	bool _isEwasmRun)
{
	bool success = true;
	m_gasCostFailure = false;

	if (_isEwasmRun)
	{
		soltestAssert(_isYulRun, "");
		selectVM(evmc_capabilities::EVMC_CAPABILITY_EWASM);
	}
	else
		selectVM(evmc_capabilities::EVMC_CAPABILITY_EVM1);

	reset();

	m_compileViaYul = _isYulRun;
	if (_isEwasmRun)
	{
		soltestAssert(m_compileViaYul, "");
		m_compileToEwasm = _isEwasmRun;
	}

	m_canEnableYulRun = false;
	m_canEnableEwasmRun = false;

	if (_isYulRun)
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Running via Yul" << (_isEwasmRun ? " (ewasm):" : ":") << endl;

	for (TestFunctionCall& test: m_tests)
		test.reset();

	map<string, solidity::test::Address> libraries;

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
			libraries[test.call().signature] = m_contractAddress;
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
				m_gasCostFailure = true;

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
				optional<bytes> builtinOutput = m_builtins.at(test.call().signature)(test.call());
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
					m_compiler.methodIdentifiers(m_compiler.lastContractName(m_sources.mainSourceFile)).isMember(test.call().signature),
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
			test.setRawBytes(move(output));
			test.setContractABI(m_compiler.contractABI(m_compiler.lastContractName(m_sources.mainSourceFile)));
		}

		vector<string> effects;
		for (SideEffectHook const& hook: m_sideEffectHooks)
			effects += hook(test.call());
		test.setSideEffects(move(effects));

		success &= test.call().expectedSideEffects == test.call().actualSideEffects;
	}

	if (!m_testCaseWantsYulRun && _isYulRun)
	{
		m_canEnableYulRun = success;
		string message = success ?
			"Test can pass via Yul, but marked with \"compileViaYul: false.\"" :
			"Test compiles via Yul, but it gives different test results.";
		AnsiColorized(_stream, _formatted, {BOLD, success ? YELLOW : MAGENTA}) <<
			_linePrefix << endl <<
			_linePrefix << message << endl;
		return TestResult::Failure;
	}

	// Right now we have sometimes different test results in Yul vs. Ewasm.
	// The main reason is that Ewasm just returns a failure in some cases.
	// TODO: If Ewasm support got fully implemented, we could implement this in the same way as above.
	if (success && !m_testCaseWantsEwasmRun && _isEwasmRun)
	{
		// TODO: There is something missing in Ewasm to support other types of revert strings:
		//  for now, we just ignore test-cases that do not use RevertStrings::Default.
		if (m_revertStrings != RevertStrings::Default)
			return TestResult::Success;

		m_canEnableEwasmRun = true;
		AnsiColorized(_stream, _formatted, {BOLD, YELLOW}) <<
			_linePrefix << endl <<
			_linePrefix << "Test can pass via Yul (Ewasm), but marked with \"compileToEwasm: false.\"" << endl;
		return TestResult::Failure;
	}

	if (!success)
	{
		// Ignore failing tests that can't yet get compiled to Ewasm:
		// if the test run was not successful and enforce compiling to ewasm was set,
		// but the test case did not want to get run with Ewasm, we just ignore this failure.
		if (m_enforceCompileToEwasm && !m_testCaseWantsEwasmRun)
			return TestResult::Success;

		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << endl;
		for (TestFunctionCall const& test: m_tests)
		{
			ErrorReporter errorReporter;
			_stream << test.format(
				errorReporter,
				_linePrefix,
				TestFunctionCall::RenderMode::ExpectedValuesExpectedGas,
				_formatted,
				/* _interactivePrint */ true
			) << endl;
			_stream << errorReporter.format(_linePrefix, _formatted);
		}
		_stream << endl;
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << endl;
		for (TestFunctionCall const& test: m_tests)
		{
			ErrorReporter errorReporter;
			_stream << test.format(
				errorReporter,
				_linePrefix,
				m_gasCostFailure ? TestFunctionCall::RenderMode::ExpectedValuesActualGas : TestFunctionCall::RenderMode::ActualValuesExpectedGas,
				_formatted,
				/* _interactivePrint */ true
			) << endl;
			_stream << errorReporter.format(_linePrefix, _formatted);
		}
		AnsiColorized(_stream, _formatted, {BOLD, RED})
			<< _linePrefix << endl
			<< _linePrefix << "Attention: Updates on the test will apply the detected format displayed." << endl;
		if (_isYulRun && m_testCaseWantsLegacyRun)
		{
			_stream << _linePrefix << endl << _linePrefix;
			AnsiColorized(_stream, _formatted, {RED_BACKGROUND}) << "Note that the test passed without Yul.";
			_stream << endl;
		}
		else if (!_isYulRun && m_testCaseWantsYulRun)
			AnsiColorized(_stream, _formatted, {BOLD, YELLOW})
				<< _linePrefix << endl
				<< _linePrefix << "Note that the test also has to pass via Yul." << endl;
		return TestResult::Failure;
	}

	return TestResult::Success;
}

bool SemanticTest::checkGasCostExpectation(TestFunctionCall& io_test, bool _compileViaYul) const
{
	string setting =
		(_compileViaYul ? "ir"s : "legacy"s) +
		(m_optimiserSettings == OptimiserSettings::full() ? "Optimized" : "");

	// We don't check gas if enforce gas cost is not active
	// or test is run with abi encoder v1 only
	// or gas used less than threshold for enforcing feature
	// or setting is "ir" and it's not included in expectations
	// or if the called function is an isoltest builtin e.g. `smokeTest` or `storageEmpty`
	if (
		!m_enforceGasCost ||
		(
			(setting == "ir" || m_gasUsed < m_enforceGasCostMinValue || m_gasUsed >= m_gas) &&
			io_test.call().expectations.gasUsed.count(setting) == 0
		) ||
		io_test.call().kind == FunctionCall::Kind::Builtin
	)
		return true;

	solAssert(!m_runWithABIEncoderV1Only, "");

	io_test.setGasCost(setting, m_gasUsed);
	return
		io_test.call().expectations.gasUsed.count(setting) > 0 &&
		m_gasUsed == io_test.call().expectations.gasUsed.at(setting);
}

void SemanticTest::printSource(ostream& _stream, string const& _linePrefix, bool _formatted) const
{
	if (m_sources.sources.empty())
		return;

	bool outputNames = (m_sources.sources.size() - m_sources.externalSources.size() != 1 || !m_sources.sources.begin()->first.empty());

	set<string> externals;
	for (auto const& [name, path]: m_sources.externalSources)
	{
		externals.insert(name);
		string externalSource;
		if (name == path)
			externalSource = name;
		else
			externalSource = name + "=" + path.generic_string();

		if (_formatted)
			_stream << _linePrefix  << formatting::CYAN << "==== ExternalSource: " << externalSource << " ===="s << formatting::RESET << endl;
		else
			_stream << _linePrefix << "==== ExternalSource: " << externalSource << " ===="s << endl;
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
							<< " ====" << formatting::RESET << endl;

				vector<char const*> sourceFormatting(source.length(), formatting::RESET);
				_stream << _linePrefix << sourceFormatting.front() << source.front();
				for (size_t i = 1; i < source.length(); i++)
				{
					if (sourceFormatting[i] != sourceFormatting[i - 1])
						_stream << sourceFormatting[i];
					if (source[i] != '\n')
						_stream << source[i];
					else
					{
						_stream << formatting::RESET << endl;
						if (i + 1 < source.length())
							_stream << _linePrefix << sourceFormatting[i];
					}
				}
				_stream << formatting::RESET;
			}
			else
			{
				if (outputNames)
					_stream << _linePrefix << "==== Source: " + name << " ====" << endl;
				stringstream stream(source);
				string line;
				while (getline(stream, line))
					_stream << _linePrefix << line << endl;
			}
		}
}

void SemanticTest::printUpdatedExpectations(ostream& _stream, string const&) const
{
	for (TestFunctionCall const& test: m_tests)
		_stream << test.format(
			"",
			m_gasCostFailure ? TestFunctionCall::RenderMode::ExpectedValuesActualGas : TestFunctionCall::RenderMode::ActualValuesExpectedGas,
			/* _highlight = */ false
		) << endl;
}

void SemanticTest::printUpdatedSettings(ostream& _stream, string const& _linePrefix)
{
	auto& settings = m_reader.settings();
	if (settings.empty() && !m_canEnableYulRun)
		return;

	_stream << _linePrefix << "// ====" << endl;
	if (m_canEnableEwasmRun)
	{
		soltestAssert(m_canEnableYulRun || m_testCaseWantsYulRun, "");
		string compileViaYul = m_reader.stringSetting("compileViaYul", "");
		if (!compileViaYul.empty())
			_stream << _linePrefix << "// compileViaYul: " << compileViaYul << "\n";
		_stream << _linePrefix << "// compileToEwasm: also\n";
	}
	else if (m_canEnableYulRun)
		_stream << _linePrefix << "// compileViaYul: also\n";

	for (auto const& [settingName, settingValue]: settings)
		if (
			!(settingName == "compileToEwasm" && m_canEnableEwasmRun) &&
			!(settingName == "compileViaYul" && (m_canEnableYulRun || m_canEnableEwasmRun))
		)
			_stream << _linePrefix << "// " << settingName << ": " << settingValue<< endl;
}

void SemanticTest::parseExpectations(istream& _stream)
{
	m_tests += TestFileParser{_stream, m_builtins}.parseFunctionCalls(m_lineOffset);
}

bool SemanticTest::deploy(
	string const& _contractName,
	u256 const& _value,
	bytes const& _arguments,
	map<string, solidity::test::Address> const& _libraries
)
{
	auto output = compileAndRunWithoutCheck(m_sources.sources, _value, _contractName, _arguments, _libraries, m_sources.mainSourceFile);
	return !output.empty() && m_transactionSuccessful;
}
