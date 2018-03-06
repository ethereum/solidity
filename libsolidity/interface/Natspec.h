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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Takes the parsed AST and produces the Natspec documentation:
 * https://github.com/ethereum/wiki/wiki/Ethereum-Natural-Specification-Format
 *
 * Can generally deal with JSON files
 */

#pragma once

#include <string>
#include <memory>
#include <json/json.h>
#include <boost/algorithm/string.hpp>

namespace dev
{
namespace solidity
{

// Forward declarations
class ContractDefinition;
struct DocTag;

class Natspec
{
public:
	/// Get the User documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A JSON representation of the contract's user documentation
	static Json::Value userDocumentation(ContractDefinition const& _contractDef);
	/// Generates the Developer's documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A JSON representation
	///                     of the contract's developer documentation
	static Json::Value devDocumentation(ContractDefinition const& _contractDef);

private:
	/// @returns concatenation of all content under the given tag name.
	static std::string extractDoc(std::multimap<std::string, DocTag> const& _tags, std::string const& _name);

	/// Helper-function that will retrieve docTags from fun->annotation().docTags,
	/// so T need to satisfy the concept for "annotation().docTags".
	/// @param _json JSON object where parsed annotations will be stored
	/// @param _fun T const* that is able satisfy annotation().docTags.
	template<typename T>
	static void extractDevAnnotations(Json::Value &_json, T const *_fun)
	{
		if (_fun)
		{
			auto dev = extractDoc(_fun->annotation().docTags, "dev");
			if (!dev.empty())
				_json["details"] = Json::Value(dev);

			auto author = extractDoc(_fun->annotation().docTags, "author");
			if (!author.empty())
				_json["author"] = author;

			auto ret = extractDoc(_fun->annotation().docTags, "return");
			if (!ret.empty())
				_json["return"] = ret;

			Json::Value params(Json::objectValue);
			auto paramRange = _fun->annotation().docTags.equal_range("param");
			for (auto i = paramRange.first; i != paramRange.second; ++i)
				params[i->second.paramName] = Json::Value(i->second.content);

			if (!params.empty())
				_json["params"] = params;
		}
	}
};

} //solidity NS
} // dev NS
