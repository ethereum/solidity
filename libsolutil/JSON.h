// SPDX-License-Identifier: GPL-3.0
/** @file JSON.h
 * @date 2016
 *
 * JSON related helpers
 */

#pragma once

#include <json/json.h>

#include <string>

namespace solidity::util {

/// Serialise the JSON object (@a _input) with indentation
std::string jsonPrettyPrint(Json::Value const& _input);

/// Serialise the JSON object (@a _input) without indentation
std::string jsonCompactPrint(Json::Value const& _input);

/// Parse a JSON string (@a _input) with enabled strict-mode and writes resulting JSON object to (@a _json)
/// \param _input JSON input string
/// \param _json [out] resulting JSON object
/// \param _errs [out] Formatted error messages
/// \return \c true if the document was successfully parsed, \c false if an error occurred.
bool jsonParseStrict(std::string const& _input, Json::Value& _json, std::string* _errs = nullptr);

}
