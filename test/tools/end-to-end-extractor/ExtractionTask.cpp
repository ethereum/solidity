#include "ExtractionTask.h"
#include "EndToEndCppTestFile.h"
#include <boost/algorithm/string.hpp>
#include <test/libsolidity/util/BytesUtils.h>

void ExtractionTask::analyse()
{
	m_task();

	std::stringstream stream;
	for (auto const &item : m_expectations)
		stream << item << std::endl;

	solidity::frontend::test::TestFileParser parser(stream);
	try
	{
		m_functionCalls = parser.parseFunctionCalls(0);
	}
	catch (...)
	{
		extractionNotPossible("error while parsing expectation");
	}
}

void ExtractionTask::extract()
{
	assert(m_sources.size() == 1);
	assert(!m_expectations.empty());
	assert(!m_functionCalls.empty());

	std::ofstream stream("/tmp/e2ev2/" + m_name + ".sol");
	for (auto const &comment : m_highlevelComments)
		stream << "// " << comment << std::endl;
	stream << boost::trim_copy(*m_sources.begin());
	stream << std::endl;

	if (!m_expectationComments.empty())
		stream << std::endl << "// found expectation comments:" << std::endl;

	for (auto &expectationComments : m_expectationComments)
	{
		const std::string &comment = expectationComments.first;
		const std::string &context = expectationComments.second;
		stream << "// " << comment << " @ " << context << std::endl;
	}

	if (!m_expectationComments.empty())
		stream << std::endl;

	if (m_alsoViaYul)
	{
		stream << "// ====" << std::endl;
		stream << "// compileViaYul: also" << std::endl;
	}

	stream << "// ----" << std::endl;
	for (auto const &call : m_functionCalls)
	{
		std::string params;
		for (auto const &param : call.arguments.parameters)
		{
			params += param.rawString;
			if (&param != &(*call.arguments.parameters.crbegin()))
				params += ", ";
		}


		std::string expectation;
		for (auto const &exp : call.expectations.result)
		{
			expectation += exp.rawString;
			if (&exp != &(*call.expectations.result.crbegin()))
				expectation += ", ";
		}
		stream << "// " << call.signature << (params.empty() ? "" : ": ") << params << " -> " << expectation;
		if (!call.expectations.comment.empty())
			stream << " # " << call.expectations.comment << " #";
		stream << std::endl;
	}
	stream << std::endl;
}
