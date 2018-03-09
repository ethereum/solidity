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

#include <test/libsolidity/AnalysisFramework.h>
#include <test/libsolidity/SyntaxTestParser.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#ifdef __unix__
#include <unistd.h>
#include <termios.h>
#endif

using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace color
{
	bool noColor = false;
	char const* red() { return noColor ? "" : "\033[1;31m"; }
	char const* green() { return noColor ? "" : "\033[1;32m"; }
	char const* yellow() { return noColor ? "" : "\033[1;33m"; }
	char const* cyan() { return noColor ? "" : "\033[1;36m"; }
	char const* bold() { return noColor ? "" : "\033[1m"; }
	char const* inverse() { return noColor ? "" : "\033[7m"; }
	char const* reset() { return noColor ? "" : "\033[0m"; }
}

#ifdef __unix__
class DisableConsoleBuffering
{
public:
	DisableConsoleBuffering()
	{
		if (tcgetattr(0, &m_termios) < 0)
			perror("tcsetattr()");
		m_termios.c_lflag &= ~ICANON;
		m_termios.c_lflag &= ~ECHO;
		m_termios.c_cc[VMIN] = 1;
		m_termios.c_cc[VTIME] = 0;
		if (tcsetattr(0, TCSANOW, &m_termios) < 0)
			perror("tcsetattr ICANON");
	}
	~DisableConsoleBuffering()
	{
		m_termios.c_lflag |= ICANON;
		m_termios.c_lflag |= ECHO;
		if (tcsetattr(0, TCSADRAIN, &m_termios) < 0)
			perror ("tcsetattr ~ICANON");
	}
private:
	struct termios m_termios;
};
#else
class DisableConsoleBuffering {};
#endif

class SyntaxTestTool: test::AnalysisFramework
{
public:
	SyntaxTestTool(string const& _name, fs::path const& _path)
		: m_name(_name), m_path(_path)
	{}

	bool process();

	static bool processPath(fs::path const& _basepath, fs::path const& _path);

	static string editor;
private:
	static string errorMessage(Error const& _e)
	{
		if (_e.comment())
			return boost::replace_all_copy(*_e.comment(), "\n", "\\n");
		else
			return "NONE";
	}
	static int getLineNumber(string const& _source, int _location);

	void printContract() const;
	void printExpected() const;
	void printErrorList(ErrorList const& _errorList, bool const _parserError) const;

	string const m_name;
	fs::path const m_path;
	SyntaxTest m_test;
};

string SyntaxTestTool::editor;

int SyntaxTestTool::getLineNumber(string const& _source, int _location)
{
	// parseAnalyseAndReturnError(...) prepends a version pragma
	_location -= strlen("pragma solidity >=0.0;\n");
	if (_location < 0)
		return -1;
	else
	{
		int line = 1;
		if (static_cast<size_t>(_location) >= _source.size())
			return -1;
		for (int i = 0; i < _location; i++)
			if (_source[i] == '\n')
				++line;
		return line;
	}
}

void SyntaxTestTool::printContract() const
{
	stringstream stream(m_test.source);
	string line;
	cout << color::cyan();
	while (getline(stream, line))
	{
		cout << "    " << line << endl;
	}
	cout << color::reset() << endl;
}

void SyntaxTestTool::printExpected() const
{
	if (m_test.expectations.empty())
		cout << color::green() << "    Success" << color::reset() << endl;
	else
		for (auto const &expectation: m_test.expectations)
		{
			cout << (expectation.type == "Warning" ? color::yellow() : color::red())
				 << "    " << expectation.type << ": " << expectation.message
				 << color::reset() << endl;
		}
}

void SyntaxTestTool::printErrorList(ErrorList const& _errorList, bool const _parserError) const
{
	if (_errorList.empty())
		cout << color::green() << "    Success" << color::reset() << endl;
	else
		for (auto &error: _errorList)
		{
			if (error->typeName() == "Warning")
			{
				if (_parserError) continue;
				cout << color::yellow();
			}
			else
				cout << color::red();

			cout << "    ";
			if (_parserError)
			{
				int line = getLineNumber(
					m_test.source,
					boost::get_error_info<errinfo_sourceLocation>(*error)->start
				);
				if (line >= 0)
					cout << "(" << line << "): ";
			}
			cout << error->typeName() << ": " << errorMessage(*error) << color::reset() << endl;
		}
}

bool SyntaxTestTool::process()
{
	bool match = true;
	bool parserError = false;
	ErrorList errorList;

	m_test = SyntaxTestParser().parse(m_path.string());

	try
	{
		errorList = parseAnalyseAndReturnError(m_test.source, true, true, true).second;

		if (errorList.size() != m_test.expectations.size())
			match = false;
		else
		{
			for (size_t i = 0; i < errorList.size(); i++)
			{
				if (!(errorList[i]->typeName() == m_test.expectations[i].type) ||
					!(errorMessage(*errorList[i]) == m_test.expectations[i].message))
				{
					match = false;
					break;
				}
			}
		}
	}
	catch (...)
	{
		match = false;
		parserError = true;
		errorList = m_compiler.errors();
	}

	cout << color::bold() << m_name << ": " << color::reset();

	if (match)
		cout << color::green() << "OK" << color::reset() << endl;
	else
	{
		cout << color::red() << "FAILED" << color::reset() << endl;

		cout << "  Contract:" << endl;
		printContract();

		if (parserError)
		{
			cout << "  " << color::inverse() << color::red() << "Parsing failed:" << color::reset() << endl;
			printErrorList(errorList, true);
		}
		else
		{
			cout << "  Expected result:" << endl;
			printExpected();
			cout << "  Obtained result:" << endl;
			printErrorList(errorList, false);
		}
		cout << endl;

		if (parserError)
			cout << "(e)dit/(s)kip/(q)uit? ";
		else
			cout << "(e)dit/(r)eplace/(s)kip/(q)uit? ";
		cout.flush();

		bool done = false;
		while (!done)
		{
			switch(cin.get())
			{
				case 's':
					done = true;
					break;
				case 'r':
					if (!parserError)
					{
						ofstream file(m_path.string(), ios::trunc);

						file << m_test.source;
						file << "// ----" << endl;
						for (auto &error: errorList)
							file << "// " << error->typeName() << ": " << errorMessage(*error) << endl;

						done = true;
					}
					break;
				case 'e':
					cout << endl << endl;
					if (system((editor + " " + m_path.string()).c_str()))
						cerr << color::red() << "Error running editor command." << color::reset() << endl << endl;
					return process();
				case 'q':
					cout << endl;
					return false;
				default:
					break;
			}
		}
		cout << endl << endl;
	}
	return true;

}

bool SyntaxTestTool::processPath(fs::path const& _basepath, fs::path const& _path)
{
	fs::path fullpath = _basepath / _path;
	if (fs::is_directory(fullpath))
	{
		for (auto const& entry: boost::iterator_range<fs::directory_iterator>(
			fs::directory_iterator(fullpath),
			fs::directory_iterator()
		))
			if (!processPath(_basepath, _path / entry.path().filename()))
				return false;
	}
	else
	{
		SyntaxTestTool testTool(_path.string(), fullpath);
		if (!testTool.process())
			return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
	DisableConsoleBuffering disableConsoleBuffering;
	(void)disableConsoleBuffering;

	if (getenv("EDITOR"))
		SyntaxTestTool::editor = getenv("EDITOR");

	fs::path testPath;
	po::options_description options(
		R"(syntaxTestTool, tool for managing syntax tests.
Usage: syntaxTestTool [Options] --testpath path
Interactively validates syntax test files.

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);
	options.add_options()
		("help", "Show this help screen.")
		("testpath", po::value<fs::path>(&testPath)->required(), "path to test files")
		(
			"no-color",
			po::bool_switch(&color::noColor)->default_value(false),
			"use colors (must be supported by the terminal)"
		)
		("editor", po::value<string>(&SyntaxTestTool::editor), "editor for opening contracts");

	po::variables_map arguments;
	try
	{
		po::command_line_parser cmdLineParser(argc, argv);
		cmdLineParser.options(options);
		po::store(cmdLineParser.run(), arguments);
		po::notify(arguments);
	}
	catch (po::error const& _exception)
	{
		cerr << _exception.what() << endl;
		return 1;
	}

	if (arguments.count("help"))
		cout << options;
	else if (fs::exists(testPath) && fs::is_directory(testPath))
		SyntaxTestTool::processPath(testPath / "libsolidity", "syntaxTests");
	else
	{
		cerr << "test path does not exist" << endl;
		return 1;
	}

	return 0;
}
