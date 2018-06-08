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

#pragma once

#include <test/libsolidity/FormattedScope.h>
#include <test/libsolidity/SolidityExecutionFramework.h>
#include <test/libsolidity/TestCase.h>
#include <libsolidity/interface/Exceptions.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace dev
{
namespace solidity
{
namespace test
{

class SemanticsTest: SolidityExecutionFramework, public TestCase
{
public:
	static std::unique_ptr<TestCase> create(std::string const& _filename)
	{ return std::unique_ptr<TestCase>(new SemanticsTest(_filename)); }
	SemanticsTest(std::string const& _filename);
	virtual ~SemanticsTest() {}

	virtual bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

	virtual void printSource(std::ostream &_stream, std::string const &_linePrefix = "", bool const _formatted = false) const override;
	virtual void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;

	/// Represents the format used to represent a range of bytes in the test expectations.
	/// Used to attempt to reformat mismatching results using the same format as the expectations.
	struct ByteRangeFormat
	{
		enum Type {
			Bool,
			String,
			Dec,
			Hash,
			Hex,
			HexString,
			SignedDec
		};

		/// Unpadded length of the byte range.
		std::size_t length;
		/// Encoding type of the byte range.
		Type type;
		/// true, if the byte range was padded.
		bool padded;
		/// @returns true, if the byte range is padded to the left, false otherwise.
		bool padsLeft() const;
		/// Tries to format the byte range from @arg _it to @arg _end.
		/// @returns A string representing the byte range or the empty string if formatting failed.
		std::string tryFormat(bytes::const_iterator _it, bytes::const_iterator _end) const;
	};

	/// Converts @arg _bytes to a formatted string representation.
	/// Attempts to use the format given in @arg _formatList, if possible, and uses
	/// generic formatting as fallback.
	static std::string bytesToString(
		bytes const& _bytes,
		std::vector<ByteRangeFormat> const& _formatList
	);
	/// Parses the bytes encoded in the comma separated string @arg _list.
	/// If @arg _formatList is not nullptr, the formatting of the string is appended.
	/// If @arg _padded is true (default), 32-byte padding is used for all elements of the string.
	/// Otherwise no padding is applied (used for "unpadded(...)" blocks).
	static bytes stringToBytes(
		std::string _list,
		std::vector<ByteRangeFormat>* _formatList = nullptr,
		bool padded = true
	);

	static std::string ipcPath;
private:
	void printCalls(
		bool _actualResults,
		std::ostream& _stream,
		std::string const& _linePrefix,
		bool const _formatted = false
	) const;

	struct SemanticsTestFunctionCall
	{
		std::string signature;
		std::string arguments;
		bytes argumentBytes;
		u256 value;
		std::string expectedResult;
		bytes expectedBytes;
		std::vector<ByteRangeFormat> expectedFormat;
	};

	void parseExpectations(std::istream &_stream);

	std::map<std::string, dev::test::Address> m_libraryAddresses;
	std::vector<std::function<void(void)>> m_initializations;
	bool m_hasDeploy = false;
	std::string m_source;
	std::vector<SemanticsTestFunctionCall> m_calls;
	std::vector<bytes> m_results;
};

}
}
}
