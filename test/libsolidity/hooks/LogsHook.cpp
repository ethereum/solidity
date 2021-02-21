#include <test/libsolidity/hooks/LogsHook.h>
#include <test/libsolidity/SemanticTest.h>
#include <test/libsolidity/SolidityExecutionFramework.h>
#include <test/libsolidity/util/SoltestErrors.h>
#include <libsolutil/AnsiColorized.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/FixedHash.h>

using namespace std;
using namespace solidity::util;

namespace solidity::frontend::test
{

LogsHook::LogsHook(SemanticTest* _test): m_test(_test) {
	using namespace std::placeholders;

	m_executionFramework = dynamic_cast<SolidityExecutionFramework*>(m_test);
	soltestAssert(m_executionFramework != nullptr, "");

	m_test->addBuiltin("logs.numLogs", std::bind(&LogsHook::numLogs, this, _1));
	m_test->addBuiltin("logs.numLogTopics", std::bind(&LogsHook::numLogTopics, this, _1));
	m_test->addBuiltin("logs.logTopic", std::bind(&LogsHook::logTopic, this, _1));
	m_test->addBuiltin("logs.logAddress", std::bind(&LogsHook::logAddress, this, _1));
	m_test->addBuiltin("logs.logData", std::bind(&LogsHook::logData, this, _1));
	m_test->addBuiltin("logs.expectEvent", std::bind(&LogsHook::expectEvent, this, _1));
}

std::optional<bytes> LogsHook::numLogs(FunctionCall const&)
{
	size_t result = m_executionFramework->numLogs();
	bytes r = util::toBigEndian(u256{result});
	return r;
}

std::optional<bytes> LogsHook::numLogTopics(FunctionCall const& _call)
{
	assert(_call.arguments.parameters.size() == 1);
	size_t logCount = m_executionFramework->numLogs();
	// todo: hex strings not supported by lexical_cast<..>(..)
	auto logIdx = boost::lexical_cast<size_t>(_call.arguments.parameters.front().rawString);
	if (logCount > 0 && logIdx < logCount)
	{
		m_touchedLogs[&_call].insert(logIdx);
		return util::toBigEndian(u256{m_executionFramework->numLogTopics(logIdx)});
	}
	return std::nullopt;
}

std::optional<bytes> LogsHook::logTopic(FunctionCall const& _call)
{
	assert(_call.arguments.parameters.size() == 2);
	auto logIdx = boost::lexical_cast<size_t>(_call.arguments.parameters.front().rawString);
	m_touchedLogs[&_call].insert(logIdx);
	auto topicIdx = boost::lexical_cast<size_t>(_call.arguments.parameters.back().rawString);
	size_t logCount = m_executionFramework->numLogs();
	// todo: hex strings not supported by lexical_cast<..>(..)
	if (logCount > 0 && logIdx < logCount)
	{
		size_t topicCount = m_executionFramework->numLogTopics(logIdx);
		if (topicCount > 0 && topicIdx < topicCount)
			return util::toBigEndian(u256{m_executionFramework->logTopic(logIdx, topicIdx)});
	}
	return std::nullopt;
}

std::optional<bytes> LogsHook::logAddress(FunctionCall const& _call)
{
	// logAddress(uint256)
	assert(_call.arguments.parameters.size() == 1);
	size_t logCount = m_executionFramework->numLogs();
	// todo: hex strings not supported by lexical_cast<..>(..)
	auto logIdx = boost::lexical_cast<size_t>(_call.arguments.parameters.front().rawString);
	m_touchedLogs[&_call].insert(logIdx);
	if (logCount > 0 && logIdx < logCount)
		return util::toBigEndian(u256{u160{m_executionFramework->logAddress(logIdx)}});
	return std::nullopt;
}

std::optional<bytes> LogsHook::logData(FunctionCall const& _call)
{
	// logData(uint256)
	assert(_call.arguments.parameters.size() == 1);
	size_t logCount = m_executionFramework->numLogs();
	// todo: hex strings not supported by lexical_cast<..>(..)
	auto logIdx = boost::lexical_cast<size_t>(_call.arguments.parameters.front().rawString);
	m_touchedLogs[&_call].insert(logIdx);
	if (logCount > 0 && logIdx < logCount)
		return m_executionFramework->logData(logIdx);
	return std::nullopt;
}

std::optional<bytes> LogsHook::expectEvent(FunctionCall const& _call)
{
	// expectEvent(uint256,string): logIdx, eventSignature
	assert(_call.arguments.parameters.size() == 2);
	size_t logCount = m_executionFramework->numLogs();
	// todo: hex strings not supported by lexical_cast<..>(..)
	auto logIdx = boost::lexical_cast<size_t>(_call.arguments.parameters.front().rawString);
	m_touchedLogs[&_call].insert(logIdx);
	auto logSignature = _call.arguments.parameters.back().rawString;
	assert(logSignature.length() >= 2);
	logSignature = logSignature.substr(1, logSignature.length() - 2);
	h256 logSignatureHash{util::keccak256(logSignature)};
	if (logCount > 0 && logIdx < logCount)
	{
		vector<h256> topics;
		size_t topicCount = m_executionFramework->numLogTopics(logIdx);
		for (size_t topicIdx = 0; topicIdx < topicCount; ++topicIdx)
			topics.push_back(m_executionFramework->logTopic(logIdx, topicIdx));
		// remove topics[0], if the signature matches.
		if (!topics.empty() && topics[0] == logSignatureHash)
			topics.erase(topics.begin());
		bytes result;
		for (auto& topic: topics)
			result += util::toBigEndian(topic);
		result += m_executionFramework->logData(logIdx);
		return result;
	}
	return std::nullopt;
}

void LogsHook::beginTestCase()
{
	m_producedLogs.clear();
	m_touchedLogs.clear();
	m_previousCall = nullptr;
}

void LogsHook::afterFunctionCall(TestFunctionCall const& _call)
{
	m_producedLogs[&_call.call()] = m_executionFramework->recordedLogs();
	m_previousCalls[&_call] = m_previousCall;
	m_previousCall = &_call;

	TestFunctionCall const* producer = m_previousCalls[&_call];
	std::vector<FunctionCall const*> consumers{};

	// Only non-builtins are able to produce logs.
	// So lets search from the current call up to the first non-builtin.
	while (producer != nullptr && producer->call().kind == FunctionCall::Kind::Builtin)
		if (m_previousCalls[producer] == nullptr)
			break;
		else
		{
			// On the way up to the producer we track all builtins that where on the way.
			// Only builtins can consume logs, we store them in the consumers vector.
			consumers.emplace_back(&producer->call());
			producer = m_previousCalls[producer];
		}

	// Producer will now point to the call that probably produced a log.
	if (producer)
	{
		consumers.emplace_back(&_call.call());
		// We iterate through the consumers to find out what logs they have consumed.
		for (auto& consumer: consumers)
			for (auto logIdx: m_touchedLogs[consumer])
				// All logs that where touched by the consumer, will be marked as
				// touched within the producer.
				m_touchedLogs[&producer->call()].insert(logIdx);
	}
}

bool LogsHook::verifyFunctionCall(const TestFunctionCall& _call)
{
	if (_call.call().kind == FunctionCall::Kind::Builtin)
		// Builtins can not produce events, so everything is ok here.
		return true;
	else
		// If not all produced logs where consumed, indicate an error by returning false.
		// But it is totally ok if more logs where consumed than produced.
		return m_touchedLogs[&_call.call()].size() >= m_producedLogs[&_call.call()].size();
}

string LogsHook::formatFunctionCall(
	const TestFunctionCall& _call,
	ErrorReporter& _errorReporter,
	std::string const& _linePrefix,
	bool const _renderResult,
	bool const _highlight) const
{
	(void) _errorReporter;
	(void) _renderResult;

	if (_call.call().kind == FunctionCall::Kind::Builtin)
		// Builtins can not produce events, so everything is ok here.
		return "";

	stringstream stream;
	set<size_t> touchedLogs{m_touchedLogs.at(&_call.call())};
	vector<LogRecord> producedLogs{m_producedLogs.at(&_call.call())};
	if (touchedLogs.size() < producedLogs.size())
	{
		std::vector<LogRecord> forgottenLogs{};
		for (auto& log: producedLogs)
			if (touchedLogs.count(log.index) == 0)
				forgottenLogs.push_back(log);

		if (!forgottenLogs.empty())
		{
			stream << std::endl;
			for (auto& log: forgottenLogs)
			{
				stream << _linePrefix;
				AnsiColorized(stream, _highlight, {util::formatting::RED_BACKGROUND})
					<< "// logs.numLogTopics: " << log.index << " -> " << log.topics.size();
				if (log != (*forgottenLogs.rbegin()))
					stream << std::endl;
				AnsiColorized(stream, _highlight, {util::formatting::RESET});
			}
		}
	}
	return stream.str();
}

} // namespace solidity::frontend::test
