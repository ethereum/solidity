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
// SPDX-License-Identifier: GPL-3.0

#include <test/libyul/YulOptimizerAssemblyTest.h>
#include <test/libyul/YulOptimizerTestCommon.h>

#include <test/libyul/Common.h>
#include <test/Common.h>

#include <libyul/Object.h>
#include <libyul/AsmPrinter.h>

#include <liblangutil/SourceReferenceFormatter.h>
#include <liblangutil/Scanner.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/CommonIO.h>

#include <libyul/YulStack.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/EthAssemblyAdapter.h>
#include <libyul/backends/evm/EVMObjectCompiler.h>

#include <libevmasm/Assembly.h>

#include <fstream>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

YulOptimizerAssemblyTest::YulOptimizerAssemblyTest(std::string const& _filename):
	EVMVersionRestrictedTestCase(_filename)
{
	boost::filesystem::path path(_filename);

	if (path.empty() || std::next(path.begin()) == path.end() || std::next(std::next(path.begin())) == path.end())
		BOOST_THROW_EXCEPTION(std::runtime_error("Filename path has to contain a directory: \"" + _filename + "\"."));
	m_optimizerStep = std::prev(std::prev(path.end()))->string();

	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult YulOptimizerAssemblyTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	std::shared_ptr<Object> object;
	std::shared_ptr<AsmAnalysisInfo> analysisInfo;
	Parser::DebugAttributeCache::Ptr debugAttributeCache = std::make_shared<Parser::DebugAttributeCache>();

	Dialect const& dialect = test::dialect("evm", solidity::test::CommonOptions::get().evmVersion());
	std::tie(object, analysisInfo) = test::parse(_stream, _linePrefix, _formatted, m_source, dialect, debugAttributeCache);
	if (!object)
		return TestResult::FatalError;

	object->analysisInfo = analysisInfo;
	YulOptimizerTestCommon tester(object, dialect);
	tester.setStep(m_optimizerStep);

	if (!tester.runStep())
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Invalid optimizer step: " << m_optimizerStep << std::endl;
		return TestResult::FatalError;
	}

	auto const printed = (object->subObjects.empty() ? AsmPrinter{ dialect }(*object->code) : object->toString(&dialect));

	m_obtainedResult = "step: " + m_optimizerStep + "\n\n" + printed + "\n";

	*object->analysisInfo = AsmAnalyzer::analyzeStrictAssertCorrect(dialect, *object);

	evmasm::Assembly assembly{solidity::test::CommonOptions::get().evmVersion(), false, {}};
	EthAssemblyAdapter adapter(assembly);
	EVMObjectCompiler::compile(
		*object,
		adapter,
		EVMDialect::strictAssemblyForEVMObjects(solidity::test::CommonOptions::get().evmVersion()),
		false, // optimize - don't run any other optimization
		solidity::test::CommonOptions::get().eofVersion()
	);

	m_obtainedResult += "\nAssembly:\n" + toString(assembly);

	return checkResult(_stream, _linePrefix, _formatted);
}
