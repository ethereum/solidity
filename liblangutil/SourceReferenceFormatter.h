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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Formatting functions for errors referencing positions and locations in the source.
 */

#pragma once

#include <ostream>
#include <sstream>
#include <functional>

namespace dev
{
struct Exception; // forward
}

namespace langutil
{
struct SourceLocation;
class Scanner;

class SourceReferenceFormatter
{
public:
	using ScannerFromSourceNameFun = std::function<langutil::Scanner const&(std::string const&)>;

	explicit SourceReferenceFormatter(
		std::ostream& _stream,
		ScannerFromSourceNameFun _scannerFromSourceName
	):
		m_stream(_stream),
		m_scannerFromSourceName(std::move(_scannerFromSourceName))
	{}

	/// Prints source location if it is given.
	void printSourceLocation(langutil::SourceLocation const* _location);
	void printExceptionInformation(dev::Exception const& _exception, std::string const& _name);

	static std::string formatExceptionInformation(
		dev::Exception const& _exception,
		std::string const& _name,
		ScannerFromSourceNameFun const& _scannerFromSourceName
	)
	{
		std::ostringstream errorOutput;

		SourceReferenceFormatter formatter(errorOutput, _scannerFromSourceName);
		formatter.printExceptionInformation(_exception, _name);
		return errorOutput.str();
	}
private:
	/// Prints source name if location is given.
	void printSourceName(langutil::SourceLocation const* _location);

	std::ostream& m_stream;
	ScannerFromSourceNameFun m_scannerFromSourceName;
};

}
