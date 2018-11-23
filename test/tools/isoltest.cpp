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

#include <libdevcore/CommonIO.h>

#include <test/Common.h>
#include <test/libsolidity/AnalysisFramework.h>
#include <test/libsolidity/SyntaxTest.h>
#include <test/libsolidity/ASTJSONTest.h>
#include <test/libsolidity/SMTCheckerJSONTest.h>
#include <test/libyul/YulOptimizerTest.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <queue>

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace dev::solidity::test::formatting;
using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

struct TestStats
{
	int successCount;
	int testCount;
	operator bool() const { return successCount == testCount; }
	TestStats& operator+=(TestStats const& _other) noexcept
	{
		successCount += _other.successCount;
		testCount += _other.testCount;
		return *this;
	}
};

class TestTool
{
public:
	TestTool(
		TestCase::TestCaseCreator _testCaseCreator,
		string const& _name,
		fs::path const& _path,
		bool _formatted
	): m_testCaseCreator(_testCaseCreator), m_formatted(_formatted), m_name(_name), m_path(_path)
	{}

	enum class Result
	{
		Success,
		Failure,
		Exception
	};

	Result process();

	static TestStats processPath(
		TestCase::TestCaseCreator _testCaseCreator,
		fs::path const& _basepath,
		fs::path const& _path,
		bool const _formatted
	);

	static string editor;
private:
	enum class Request
	{
		Skip,
		Rerun,
		Quit
	};

	Request handleResponse(bool const _exception);

	TestCase::TestCaseCreator m_testCaseCreator;
	bool const m_formatted = false;
	string const m_name;
	fs::path const m_path;
	unique_ptr<TestCase> m_test;
	static bool m_exitRequested;
};

string TestTool::editor;
bool TestTool::m_exitRequested = false;

TestTool::Result TestTool::process()
{
	bool success;
	std::stringstream outputMessages;

	(FormattedScope(cout, m_formatted, {BOLD}) << m_name << ": ").flush();

	try
	{
		m_test = m_testCaseCreator(m_path.string());
		success = m_test->run(outputMessages, "  ", m_formatted);
	}
	catch(boost::exception const& _e)
	{
		FormattedScope(cout, m_formatted, {BOLD, RED}) <<
			"Exception during syntax test: " << boost::diagnostic_information(_e) << endl;
		return Result::Exception;
	}
	catch (std::exception const& _e)
	{
		FormattedScope(cout, m_formatted, {BOLD, RED}) <<
			"Exception during syntax test: " << _e.what() << endl;
		return Result::Exception;
	}
	catch (...)
	{
		FormattedScope(cout, m_formatted, {BOLD, RED}) <<
			"Unknown exception during syntax test." << endl;
		return Result::Exception;
	}

	if (success)
	{
		FormattedScope(cout, m_formatted, {BOLD, GREEN}) << "OK" << endl;
		return Result::Success;
	}
	else
	{
		FormattedScope(cout, m_formatted, {BOLD, RED}) << "FAIL" << endl;

		FormattedScope(cout, m_formatted, {BOLD, CYAN}) << "  Contract:" << endl;
		m_test->printSource(cout, "    ", m_formatted);

		cout << endl << outputMessages.str() << endl;
		return Result::Failure;
	}
}

TestTool::Request TestTool::handleResponse(bool const _exception)
{
	if (_exception)
		cout << "(e)dit/(s)kip/(q)uit? ";
	else
		cout << "(e)dit/(u)pdate expectations/(s)kip/(q)uit? ";
	cout.flush();

	while (true)
	{
		switch(readStandardInputChar())
		{
		case 's':
			cout << endl;
			return Request::Skip;
		case 'u':
			if (_exception)
				break;
			else
			{
				cout << endl;
				ofstream file(m_path.string(), ios::trunc);
				m_test->printSource(file);
				file << "// ----" << endl;
				m_test->printUpdatedExpectations(file, "// ");
				return Request::Rerun;
			}
		case 'e':
			cout << endl << endl;
			if (system((TestTool::editor + " \"" + m_path.string() + "\"").c_str()))
				cerr << "Error running editor command." << endl << endl;
			return Request::Rerun;
		case 'q':
			cout << endl;
			return Request::Quit;
		default:
			break;
		}
	}
}

TestStats TestTool::processPath(
	TestCase::TestCaseCreator _testCaseCreator,
	fs::path const& _basepath,
	fs::path const& _path,
	bool const _formatted
)
{
	std::queue<fs::path> paths;
	paths.push(_path);
	int successCount = 0;
	int testCount = 0;

	while (!paths.empty())
	{
		auto currentPath = paths.front();

		fs::path fullpath = _basepath / currentPath;
		if (fs::is_directory(fullpath))
		{
			paths.pop();
			for (auto const& entry: boost::iterator_range<fs::directory_iterator>(
				fs::directory_iterator(fullpath),
				fs::directory_iterator()
			))
				if (fs::is_directory(entry.path()) || TestCase::isTestFilename(entry.path().filename()))
					paths.push(currentPath / entry.path().filename());
		}
		else if (m_exitRequested)
		{
			++testCount;
			paths.pop();
		}
		else
		{
			++testCount;
			TestTool testTool(_testCaseCreator, currentPath.string(), fullpath, _formatted);
			auto result = testTool.process();

			switch(result)
			{
			case Result::Failure:
			case Result::Exception:
				switch(testTool.handleResponse(result == Result::Exception))
				{
				case Request::Quit:
					paths.pop();
					m_exitRequested = true;
					break;
				case Request::Rerun:
					cout << "Re-running test case..." << endl;
					--testCount;
					break;
				case Request::Skip:
					paths.pop();
					break;
				}
				break;
			case Result::Success:
				paths.pop();
				++successCount;
				break;
			}
		}
	}

	return { successCount, testCount };

}

namespace
{

void setupTerminal()
{
#if defined(_WIN32) && defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
	// Set output mode to handle virtual terminal (ANSI escape sequences)
	// ignore any error, as this is just a "nice-to-have"
	// only windows needs to be taken care of, as other platforms (Linux/OSX) support them natively.
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
		return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
		return;
#endif
}

boost::optional<TestStats> runTestSuite(
	string const& _name,
	fs::path const& _basePath,
	fs::path const& _subdirectory,
	TestCase::TestCaseCreator _testCaseCreator,
	bool _formatted
)
{
	fs::path testPath = _basePath / _subdirectory;

	if (!fs::exists(testPath) || !fs::is_directory(testPath))
	{
		cerr << _name << " tests not found. Use the --testpath argument." << endl;
		return {};
	}

	TestStats stats = TestTool::processPath(_testCaseCreator, _basePath, _subdirectory, _formatted);

	cout << endl << _name << " Test Summary: ";
	FormattedScope(cout, _formatted, {BOLD, stats ? GREEN : RED}) <<
		stats.successCount <<
		"/" <<
		stats.testCount;
	cout << " tests successful." << endl << endl;

	return stats;
}

}

int main(int argc, char *argv[])
{
	setupTerminal();

	if (getenv("EDITOR"))
		TestTool::editor = getenv("EDITOR");
	else if (fs::exists("/usr/bin/editor"))
		TestTool::editor = "/usr/bin/editor";

	fs::path testPath;
	bool disableSMT = false;
	bool formatted = true;
	po::options_description options(
		R"(isoltest, tool for interactively managing test contracts.
Usage: isoltest [Options] --testpath path
Interactively validates test contracts.

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);
	options.add_options()
		("help", "Show this help screen.")
		("testpath", po::value<fs::path>(&testPath), "path to test files")
		("no-smt", "disable SMT checker")
		("no-color", "don't use colors")
		("editor", po::value<string>(&TestTool::editor), "editor for opening contracts");

	po::variables_map arguments;
	try
	{
		po::command_line_parser cmdLineParser(argc, argv);
		cmdLineParser.options(options);
		po::store(cmdLineParser.run(), arguments);

		if (arguments.count("help"))
		{
			cout << options << endl;
			return 0;
		}

		if (arguments.count("no-color"))
			formatted = false;

		po::notify(arguments);

		if (arguments.count("no-smt"))
			disableSMT = true;
	}
	catch (std::exception const& _exception)
	{
		cerr << _exception.what() << endl;
		return 1;
	}

	if (testPath.empty())
		testPath = dev::test::discoverTestPath();

	TestStats global_stats{0, 0};

	// Actually run the tests.
	// If you add new tests here, you also have to add them in boostTest.cpp
	if (auto stats = runTestSuite("Syntax", testPath / "libsolidity", "syntaxTests", SyntaxTest::create, formatted))
		global_stats += *stats;
	else
		return 1;

	if (auto stats = runTestSuite("JSON AST", testPath / "libsolidity", "ASTJSON", ASTJSONTest::create, formatted))
		global_stats += *stats;
	else
		return 1;

	if (auto stats = runTestSuite(
		"Yul Optimizer",
		testPath / "libyul",
		"yulOptimizerTests",
		yul::test::YulOptimizerTest::create,
		formatted
	))
		global_stats += *stats;
	else
		return 1;

	if (!disableSMT)
	{
		if (auto stats = runTestSuite(
			"SMT Checker",
			testPath / "libsolidity",
			"smtCheckerTests",
			SyntaxTest::create,
			formatted
		))
			global_stats += *stats;
		else
			return 1;

		if (auto stats = runTestSuite(
			"SMT Checker JSON",
			testPath / "libsolidity",
			"smtCheckerTestsJSON",
			SMTCheckerTest::create,
			formatted
		))
			global_stats += *stats;
		else
			return 1;
	}

	cout << endl << "Summary: ";
	FormattedScope(cout, formatted, {BOLD, global_stats ? GREEN : RED}) <<
		 global_stats.successCount << "/" << global_stats.testCount;
	cout << " tests successful." << endl;

	return global_stats ? 0 : 1;
}
