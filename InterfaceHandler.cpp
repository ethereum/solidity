
#include <libsolidity/InterfaceHandler.h>
#include <libsolidity/AST.h>
#include <libsolidity/CompilerStack.h>

namespace dev
{
namespace solidity
{

/* -- public -- */

InterfaceHandler::InterfaceHandler()
{
	m_lastTag = DocTagType::NONE;
}

std::unique_ptr<std::string> InterfaceHandler::getDocumentation(ContractDefinition const& _contractDef,
																DocumentationType _type)
{
	switch(_type)
	{
	case DocumentationType::NATSPEC_USER:
		return getUserDocumentation(_contractDef);
	case DocumentationType::NATSPEC_DEV:
		return getDevDocumentation(_contractDef);
	case DocumentationType::ABI_INTERFACE:
		return getABIInterface(_contractDef);
	}

	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown documentation type"));
	return nullptr;
}

std::unique_ptr<std::string> InterfaceHandler::getABIInterface(ContractDefinition const& _contractDef)
{
	Json::Value methods(Json::arrayValue);

	for (auto const& it: _contractDef.getInterfaceFunctions())
	{
		Json::Value method;
		Json::Value inputs(Json::arrayValue);
		Json::Value outputs(Json::arrayValue);

		auto populateParameters = [](std::vector<ASTPointer<VariableDeclaration>> const& _vars)
		{
			Json::Value params(Json::arrayValue);
			for (ASTPointer<VariableDeclaration> const& var: _vars)
			{
				Json::Value input;
				input["name"] = var->getName();
				input["type"] = var->getType()->toString();
				params.append(input);
			}
			return params;
		};

		method["name"] = it.second->getName();
		method["constant"] = it.second->isDeclaredConst();
		method["inputs"] = populateParameters(it.second->getParameters());
		method["outputs"] = populateParameters(it.second->getReturnParameters());
		methods.append(method);
	}
	return std::unique_ptr<std::string>(new std::string(m_writer.write(methods)));
}

std::unique_ptr<std::string> InterfaceHandler::getUserDocumentation(ContractDefinition const& _contractDef)
{
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	for (auto const& it: _contractDef.getInterfaceFunctions())
	{
		Json::Value user;
		auto strPtr = it.second->getDocumentation();
		if (strPtr)
		{
			resetUser();
			parseDocString(*strPtr, CommentOwner::FUNCTION);
			if (!m_notice.empty())
			{// since @notice is the only user tag if missing function should not appear
				user["notice"] = Json::Value(m_notice);
				methods[it.second->getName()] = user;
			}
		}
	}
	doc["methods"] = methods;

	return std::unique_ptr<std::string>(new std::string(m_writer.write(doc)));
}

std::unique_ptr<std::string> InterfaceHandler::getDevDocumentation(ContractDefinition const& _contractDef)
{
	// LTODO: Somewhere in this function warnings for mismatch of param names
	// should be thrown
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	auto contractDoc = _contractDef.getDocumentation();
	if (contractDoc)
	{
		m_contractAuthor.clear();
		m_title.clear();
		parseDocString(*contractDoc, CommentOwner::CONTRACT);

		if (!m_contractAuthor.empty())
			doc["author"] = m_contractAuthor;

		if (!m_title.empty())
			doc["title"] = m_title;
	}

	for (auto const& it: _contractDef.getInterfaceFunctions())
	{
		Json::Value method;
		auto strPtr = it.second->getDocumentation();
		if (strPtr)
		{
			resetDev();
			parseDocString(*strPtr, CommentOwner::FUNCTION);

			if (!m_dev.empty())
				method["details"] = Json::Value(m_dev);

			if (!m_author.empty())
				method["author"] = m_author;

			Json::Value params(Json::objectValue);
			for (auto const& pair: m_params)
				params[pair.first] = pair.second;

			if (!m_params.empty())
				method["params"] = params;

			if (!m_return.empty())
				method["return"] = m_return;

			if (!method.empty()) // add the function, only if we have any documentation to add
				methods[it.second->getName()] = method;
		}
	}
	doc["methods"] = methods;

	return std::unique_ptr<std::string>(new std::string(m_writer.write(doc)));
}

/* -- private -- */
void InterfaceHandler::resetUser()
{
	m_notice.clear();
}

void InterfaceHandler::resetDev()
{
	m_dev.clear();
	m_author.clear();
	m_return.clear();
	m_params.clear();
}

static inline std::string::const_iterator skipLineOrEOS(std::string::const_iterator _nlPos,
														std::string::const_iterator _end)
{
	return (_nlPos == _end) ? _end : ++_nlPos;
}

std::string::const_iterator InterfaceHandler::parseDocTagLine(std::string::const_iterator _pos,
															  std::string::const_iterator _end,
															  std::string& _tagString,
															  DocTagType _tagType,
															  bool _appending)
{
	auto nlPos = std::find(_pos, _end, '\n');
	if (_appending && _pos < _end && *_pos != ' ')
		_tagString += " ";
	std::copy(_pos, nlPos, back_inserter(_tagString));
	m_lastTag = _tagType;
	return skipLineOrEOS(nlPos, _end);
}

std::string::const_iterator InterfaceHandler::parseDocTagParam(std::string::const_iterator _pos,
															   std::string::const_iterator _end)
{
	// find param name
	auto currPos = std::find(_pos, _end, ' ');
	if (currPos == _end)
		BOOST_THROW_EXCEPTION(DocstringParsingError() << errinfo_comment("End of param name not found" + std::string(_pos, _end)));


	auto paramName = std::string(_pos, currPos);

	currPos += 1;
	auto nlPos = std::find(currPos, _end, '\n');
	auto paramDesc = std::string(currPos, nlPos);
	m_params.push_back(std::make_pair(paramName, paramDesc));

	m_lastTag = DocTagType::PARAM;
	return skipLineOrEOS(nlPos, _end);
}

std::string::const_iterator InterfaceHandler::appendDocTagParam(std::string::const_iterator _pos,
																std::string::const_iterator _end)
{
	// Should never be called with an empty vector
	solAssert(!m_params.empty(), "Internal: Tried to append to empty parameter");

	auto pair = m_params.back();
	if (_pos < _end && *_pos != ' ')
		pair.second += " ";
	auto nlPos = std::find(_pos, _end, '\n');
	std::copy(_pos, nlPos, back_inserter(pair.second));

	m_params.at(m_params.size() - 1) = pair;

	return skipLineOrEOS(nlPos, _end);
}

std::string::const_iterator InterfaceHandler::parseDocTag(std::string::const_iterator _pos,
														  std::string::const_iterator _end,
														  std::string const& _tag,
														  CommentOwner _owner)
{
	// LTODO: need to check for @(start of a tag) between here and the end of line
	// for all cases. Also somehow automate list of acceptable tags for each
	// language construct since current way does not scale well.
	if (m_lastTag == DocTagType::NONE || _tag != "")
	{
		if (_tag == "dev")
			return parseDocTagLine(_pos, _end, m_dev, DocTagType::DEV, false);
		else if (_tag == "notice")
			return parseDocTagLine(_pos, _end, m_notice, DocTagType::NOTICE, false);
		else if (_tag == "return")
			return parseDocTagLine(_pos, _end, m_return, DocTagType::RETURN, false);
		else if (_tag == "author")
		{
			if (_owner == CommentOwner::CONTRACT)
				return parseDocTagLine(_pos, _end, m_contractAuthor, DocTagType::AUTHOR, false);
			else if (_owner == CommentOwner::FUNCTION)
				return parseDocTagLine(_pos, _end, m_author, DocTagType::AUTHOR, false);
			else
				// LTODO: for now this else makes no sense but later comments will go to more language constructs
				BOOST_THROW_EXCEPTION(DocstringParsingError() << errinfo_comment("@author tag is legal only for contracts"));
		}
		else if (_tag == "title")
		{
			if (_owner == CommentOwner::CONTRACT)
				return parseDocTagLine(_pos, _end, m_title, DocTagType::TITLE, false);
			else
				// LTODO: Unknown tag, throw some form of warning and not just an exception
				BOOST_THROW_EXCEPTION(DocstringParsingError() << errinfo_comment("@title tag is legal only for contracts"));
		}
		else if (_tag == "param")
			return parseDocTagParam(_pos, _end);
		else
			// LTODO: Unknown tag, throw some form of warning and not just an exception
			BOOST_THROW_EXCEPTION(DocstringParsingError() << errinfo_comment("Unknown tag " + _tag + " encountered"));
	}
	else
		return appendDocTag(_pos, _end, _owner);
}

std::string::const_iterator InterfaceHandler::appendDocTag(std::string::const_iterator _pos,
														   std::string::const_iterator _end,
														   CommentOwner _owner)
{
	switch (m_lastTag)
	{
	case DocTagType::DEV:
		return parseDocTagLine(_pos, _end, m_dev, DocTagType::DEV, true);
	case DocTagType::NOTICE:
		return parseDocTagLine(_pos, _end, m_notice, DocTagType::NOTICE, true);
	case DocTagType::RETURN:
		return parseDocTagLine(_pos, _end, m_return, DocTagType::RETURN, true);
	case DocTagType::AUTHOR:
		if (_owner == CommentOwner::CONTRACT)
			return parseDocTagLine(_pos, _end, m_contractAuthor, DocTagType::AUTHOR, true);
		else if (_owner == CommentOwner::FUNCTION)
			return parseDocTagLine(_pos, _end, m_author, DocTagType::AUTHOR, true);
		else
			// LTODO: Unknown tag, throw some form of warning and not just an exception
			BOOST_THROW_EXCEPTION(DocstringParsingError() << errinfo_comment("@author tag in illegal comment"));
	case DocTagType::TITLE:
		if (_owner == CommentOwner::CONTRACT)
			return parseDocTagLine(_pos, _end, m_title, DocTagType::TITLE, true);
		else
			// LTODO: Unknown tag, throw some form of warning and not just an exception
			BOOST_THROW_EXCEPTION(DocstringParsingError() << errinfo_comment("@title tag in illegal comment"));
	case DocTagType::PARAM:
		return appendDocTagParam(_pos, _end);
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Internal: Illegal documentation tag type"));
		break;
	}
}

static inline std::string::const_iterator getFirstSpaceOrNl(std::string::const_iterator _pos,
															std::string::const_iterator _end)
{
	auto spacePos = std::find(_pos, _end, ' ');
	auto nlPos = std::find(_pos, _end, '\n');
	return (spacePos < nlPos) ? spacePos : nlPos;
}

void InterfaceHandler::parseDocString(std::string const& _string, CommentOwner _owner)
{
	auto currPos = _string.begin();
	auto end = _string.end();

	while (currPos != end)
	{
		auto tagPos = std::find(currPos, end, '@');
		auto nlPos = std::find(currPos, end, '\n');

		if (tagPos != end && tagPos < nlPos)
		{
			// we found a tag
			auto tagNameEndPos = getFirstSpaceOrNl(tagPos, end);
			if (tagNameEndPos == end)
				BOOST_THROW_EXCEPTION(DocstringParsingError() <<
									  errinfo_comment("End of tag " + std::string(tagPos, tagNameEndPos) + "not found"));

			currPos = parseDocTag(tagNameEndPos + 1, end, std::string(tagPos + 1, tagNameEndPos), _owner);
		}
		else if (m_lastTag != DocTagType::NONE) // continuation of the previous tag
			currPos = appendDocTag(currPos, end, _owner);
		else if (currPos != end) // skip the line if a newline was found
			currPos = nlPos + 1;
	}
}

} //solidity NS
} // dev NS
