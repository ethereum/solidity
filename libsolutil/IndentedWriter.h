// SPDX-License-Identifier: GPL-3.0
/**
 * @date 2017
 * Indented text writer.
 */

#pragma once

#include <vector>
#include <string>

#include <libsolutil/Exceptions.h>

namespace solidity::util
{

DEV_SIMPLE_EXCEPTION(IndentedWriterError);

class IndentedWriter
{
public:
	// Returns the formatted output.
	std::string format() const;

	// Go one indentation level in.
	void indent();

	// Go one indentation level out.
	void unindent();

	// Add text.
	void add(std::string const& _str);

	// Add text with new line.
	void addLine(std::string const& _line);

	// Add new line.
	void newLine();

private:
	struct Line
	{
		std::string contents;
		unsigned indentation;
	};

	std::vector<Line> m_lines{{std::string(), 0}};
};

}
