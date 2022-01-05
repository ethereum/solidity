#include <libsolidity/lsp/HandlerBase.h>
#include <libsolidity/lsp/LanguageServer.h>
#include <libsolidity/lsp/Utils.h>
#include <libsolidity/ast/AST.h>

#include <liblangutil/Exceptions.h>

using namespace std;

namespace solidity::lsp
{

using namespace langutil;

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

optional<SourceLocation> HandlerBase::parsePosition(string const& _sourceUnitName, Json::Value const& _position) const
{
	if (!fileRepository().sourceUnits().count(_sourceUnitName))
		return nullopt;

	if (optional<LineColumn> lineColumn = parseLineColumn(_position))
		if (optional<int> const offset = CharStream::translateLineColumnToPosition(
			fileRepository().sourceUnits().at(_sourceUnitName),
			*lineColumn
		))
			return SourceLocation{*offset, *offset, make_shared<string>(_sourceUnitName)};
	return nullopt;
}

optional<SourceLocation> HandlerBase::parseRange(string const& _sourceUnitName, Json::Value const& _range) const
{
	if (!_range.isObject())
		return nullopt;
	optional<SourceLocation> start = parsePosition(_sourceUnitName, _range["start"]);
	optional<SourceLocation> end = parsePosition(_sourceUnitName, _range["end"]);
	if (!start || !end)
		return nullopt;
	solAssert(*start->sourceName == *end->sourceName);
	start->end = end->end;
	return start;
}

}
