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

#include <cstdint>
#include <string>
#include <tuple>

namespace langutil {

/**
 * Unidirectional stream of characters.
 *
 * This CharStream is used by lexical analyzers as the source.
 */
class CharStream
{
public:
	CharStream(): m_position(0) {}
	explicit CharStream(std::string const& _source): m_source(_source), m_position(0) {}

	int position() const { return m_position; }
	bool isPastEndOfInput(size_t _charsForward = 0) const { return (m_position + _charsForward) >= m_source.size(); }

	char get(size_t _charsForward = 0) const { return m_source[m_position + _charsForward]; }
	char advanceAndGet(size_t _chars = 1);
	char rollback(size_t _amount);

	void reset() { m_position = 0; }

	std::string const& source() const { return m_source; }

	///@{
	///@name Error printing helper functions
	/// Functions that help pretty-printing parse errors
	/// Do only use in error cases, they are quite expensive.
	std::string lineAtPosition(int _position) const;
	std::tuple<int, int> translatePositionToLineColumn(int _position) const;
	///@}

private:
	std::string m_source;
	size_t m_position;
};

}
