// SPDX-License-Identifier: GPL-3.0
/**
 * @date 2019
 * @author julius <djudju@protonmail.com>
 * Converts inline assembly AST to JSON format
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <liblangutil/SourceLocation.h>
#include <json/json.h>
#include <boost/variant.hpp>
#include <vector>

namespace solidity::yul
{

/**
 * Converter of the yul AST into JSON format
 */
class AsmJsonConverter: public boost::static_visitor<Json::Value>
{
public:
	/// Create a converter to JSON for any block of inline assembly
	/// @a _sourceIndex to be used to abbreviate source name in the source locations
	explicit AsmJsonConverter(size_t _sourceIndex): m_sourceIndex(std::to_string(_sourceIndex)) {}

	Json::Value operator()(Block const& _node) const;
	Json::Value operator()(TypedName const& _node) const;
	Json::Value operator()(Literal const& _node) const;
	Json::Value operator()(Identifier const& _node) const;
	Json::Value operator()(Assignment const& _node) const;
	Json::Value operator()(VariableDeclaration const& _node) const;
	Json::Value operator()(FunctionDefinition const& _node) const;
	Json::Value operator()(FunctionCall const& _node) const;
	Json::Value operator()(If const& _node) const;
	Json::Value operator()(Switch const& _node) const;
	Json::Value operator()(Case const& _node) const;
	Json::Value operator()(ForLoop const& _node) const;
	Json::Value operator()(Break const& _node) const;
	Json::Value operator()(Continue const& _node) const;
	Json::Value operator()(Leave const& _node) const;
	Json::Value operator()(ExpressionStatement const& _node) const;
	Json::Value operator()(Label const& _node) const;

private:
	Json::Value createAstNode(langutil::SourceLocation const& _location, std::string _nodeType) const;
	template <class T>
	Json::Value vectorOfVariantsToJson(std::vector<T> const& vec) const;

	std::string const m_sourceIndex;
};

}
