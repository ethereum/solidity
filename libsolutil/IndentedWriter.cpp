// SPDX-License-Identifier: GPL-3.0
/**
 * @date 2017
 * Indented text writer.
 */

#include <libsolutil/IndentedWriter.h>
#include <libsolutil/Assertions.h>

using namespace std;
using namespace solidity::util;

string IndentedWriter::format() const
{
	string result;
	for (auto const& line: m_lines)
		result += string(line.indentation * 4, ' ') + line.contents + "\n";
	return result;
}

void IndentedWriter::newLine()
{
	if (!m_lines.back().contents.empty())
		m_lines.emplace_back(Line{string(), m_lines.back().indentation});
}

void IndentedWriter::indent()
{
	newLine();
	m_lines.back().indentation++;
}

void IndentedWriter::unindent()
{
	newLine();
	assertThrow(m_lines.back().indentation > 0, IndentedWriterError, "Negative indentation.");
	m_lines.back().indentation--;
}

void IndentedWriter::add(string const& _str)
{
	m_lines.back().contents += _str;
}

void IndentedWriter::addLine(string const& _line)
{
	newLine();
	add(_line);
	newLine();
}
