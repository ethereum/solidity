// SPDX-License-Identifier: GPL-3.0
/**
 * Miscellaneous utilities for use in yul-phaser.
 */

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace solidity::phaser
{

/// Loads the whole file into memory, splits the content into lines, strips newlines and
/// returns the result as a list of strings.
///
/// Throws FileOpenError if the file does not exist or cannot be opened for reading.
/// Throws FileReadError if any read operation fails during the whole process.
std::vector<std::string> readLinesFromFile(std::string const& _path);

/// Reads a token from the input stream and translates it to a string using a map.
/// Sets the failbit in the stream if there's no matching value in the map.
template <typename C>
std::istream& deserializeChoice(
	std::istream& _inputStream,
	C& _choice,
	std::map<std::string, C> const& _stringToValueMap
)
{
	std::string deserializedValue;
	_inputStream >> deserializedValue;

	auto const& pair = _stringToValueMap.find(deserializedValue);
	if (pair != _stringToValueMap.end())
		_choice = pair->second;
	else
		_inputStream.setstate(std::ios_base::failbit);

	return _inputStream;
}

/// Translates a value to a string using a map and prints it to the output stream.
/// Sets the failbit if the value is not in the map.
template <typename C>
std::ostream& serializeChoice(
	std::ostream& _outputStream,
	C const& _choice,
	std::map<C, std::string> const& _valueToStringMap
)
{
	auto const& pair = _valueToStringMap.find(_choice);
	if (pair != _valueToStringMap.end())
		_outputStream << pair->second;
	else
		_outputStream.setstate(std::ios_base::failbit);

	return _outputStream;
}

}
