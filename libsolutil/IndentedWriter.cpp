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
/**
 * @date 2017
 * Indented text writer.
 */

#include <libsolutil/IndentedWriter.h>
#include <libsolutil/Assertions.h>

using namespace solidity::util;

std::string IndentedWriter::format() const
{
	std::string result;
	for (auto const& line: m_lines)
		result += std::string(line.indentation * 4, ' ') + line.contents + "\n";
	return result;
}

void IndentedWriter::newLine()
{
	if (!m_lines.back().contents.empty())
		m_lines.emplace_back(Line{std::string(), m_lines.back().indentation});
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

void IndentedWriter::add(std::string const& _str)
{
	m_lines.back().contents += _str;
}

void IndentedWriter::addLine(std::string const& _line)
{
	newLine();
	add(_line);
	newLine();
}
