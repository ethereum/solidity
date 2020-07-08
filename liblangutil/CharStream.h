// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity scanner.
 */

#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <utility>

namespace solidity::langutil
{

/**
 * Bidirectional stream of characters.
 *
 * This CharStream is used by lexical analyzers as the source.
 */
class CharStream
{
public:
	CharStream() = default;
	explicit CharStream(std::string  _source, std::string  name):
		m_source(std::move(_source)), m_name(std::move(name)) {}

	size_t position() const { return m_position; }
	bool isPastEndOfInput(size_t _charsForward = 0) const { return (m_position + _charsForward) >= m_source.size(); }

	char get(size_t _charsForward = 0) const { return m_source[m_position + _charsForward]; }
	char advanceAndGet(size_t _chars = 1);
	/// Sets scanner position to @ _amount characters backwards in source text.
	/// @returns The character of the current location after update is returned.
	char rollback(size_t _amount);
	/// Sets scanner position to @ _location if it refers a valid offset in m_source.
	/// If not, nothing is done.
	/// @returns The character of the current location after update is returned.
	char setPosition(size_t _location);

	void reset() { m_position = 0; }

	std::string const& source() const noexcept { return m_source; }
	std::string const& name() const noexcept { return m_name; }

	///@{
	///@name Error printing helper functions
	/// Functions that help pretty-printing parse errors
	/// Do only use in error cases, they are quite expensive.
	std::string lineAtPosition(int _position) const;
	std::tuple<int, int> translatePositionToLineColumn(int _position) const;
	///@}

private:
	std::string m_source;
	std::string m_name;
	size_t m_position{0};
};

}
