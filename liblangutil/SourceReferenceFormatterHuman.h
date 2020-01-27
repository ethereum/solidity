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
/**
 * Formatting functions for errors referencing positions and locations in the source.
 */

#pragma once

#include <liblangutil/SourceReferenceExtractor.h>
#include <liblangutil/SourceReferenceFormatter.h> // SourceReferenceFormatterBase

#include <libsolutil/AnsiColorized.h>

#include <ostream>
#include <sstream>
#include <functional>

namespace solidity::util
{
struct Exception; // forward
}

namespace solidity::langutil
{

struct SourceLocation;
struct SourceReference;

class SourceReferenceFormatterHuman: public SourceReferenceFormatter
{
public:
	SourceReferenceFormatterHuman(std::ostream& _stream, bool colored):
		SourceReferenceFormatter{_stream}, m_colored{colored}
	{}

	void printSourceLocation(SourceReference const& _ref) override;
	void printExceptionInformation(SourceReferenceExtractor::Message const& _msg) override;
	using SourceReferenceFormatter::printExceptionInformation;

	static std::string formatExceptionInformation(
		util::Exception const& _exception,
		std::string const& _name,
		bool colored = false
	)
	{
		std::ostringstream errorOutput;

		SourceReferenceFormatterHuman formatter(errorOutput, colored);
		formatter.printExceptionInformation(_exception, _name);
		return errorOutput.str();
	}

private:
	util::AnsiColorized normalColored() const;
	util::AnsiColorized frameColored() const;
	util::AnsiColorized errorColored() const;
	util::AnsiColorized messageColored() const;
	util::AnsiColorized secondaryColored() const;
	util::AnsiColorized highlightColored() const;
	util::AnsiColorized diagColored() const;

private:
	bool m_colored;
};

}
