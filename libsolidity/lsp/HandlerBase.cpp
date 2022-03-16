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
// SPDX-License-Identifier: GPL-3.0
#include <libsolutil/Exceptions.h>

#include <libsolidity/lsp/HandlerBase.h>
#include <libsolidity/lsp/LanguageServer.h>
#include <libsolidity/lsp/Utils.h>
#include <libsolidity/ast/AST.h>

#include <liblangutil/Exceptions.h>

#include <fmt/format.h>

using namespace solidity::langutil;
using namespace solidity::lsp;
using namespace solidity::util;
using namespace std;

Json::Value HandlerBase::toRange(SourceLocation const& _location) const
{
	if (!_location.hasText())
		return toJsonRange({}, {});

	solAssert(_location.sourceName, "");
	langutil::CharStream const& stream = charStreamProvider().charStream(*_location.sourceName);
	LineColumn start = stream.translatePositionToLineColumn(_location.start);
	LineColumn end = stream.translatePositionToLineColumn(_location.end);
	return toJsonRange(start, end);
}

Json::Value HandlerBase::toJson(SourceLocation const& _location) const
{
	solAssert(_location.sourceName);
	Json::Value item = Json::objectValue;
	item["uri"] = fileRepository().sourceUnitNameToClientPath(*_location.sourceName);
	item["range"] = toRange(_location);
	return item;
}

pair<string, LineColumn> HandlerBase::extractSourceUnitNameAndLineColumn(Json::Value const& _args) const
{
	string const uri = _args["textDocument"]["uri"].asString();
	string const sourceUnitName = fileRepository().clientPathToSourceUnitName(uri);
	if (!fileRepository().sourceUnits().count(sourceUnitName))
		BOOST_THROW_EXCEPTION(
			RequestError(ErrorCode::RequestFailed) <<
			errinfo_comment("Unknown file: " + uri)
		);

	auto const lineColumn = parseLineColumn(_args["position"]);
	if (!lineColumn)
		BOOST_THROW_EXCEPTION(
			RequestError(ErrorCode::RequestFailed) <<
			errinfo_comment(fmt::format(
				"Unknown position {line}:{column} in file: {file}",
				fmt::arg("line", lineColumn.value().line),
				fmt::arg("column", lineColumn.value().column),
				fmt::arg("file", sourceUnitName)
			))
		);

	return {sourceUnitName, *lineColumn};
}
