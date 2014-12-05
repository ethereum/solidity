
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
	m_lastTag = DOCTAG_NONE;
}

std::unique_ptr<std::string> InterfaceHandler::getDocumentation(std::shared_ptr<ContractDefinition> _contractDef,
																enum DocumentationType _type)
{
	switch(_type)
	{
	case NATSPEC_USER:
		return getUserDocumentation(_contractDef);
	case NATSPEC_DEV:
		return getDevDocumentation(_contractDef);
	case ABI_INTERFACE:
		return getABIInterface(_contractDef);
	}

	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown documentation type"));
	return nullptr;
}

std::unique_ptr<std::string> InterfaceHandler::getABIInterface(std::shared_ptr<ContractDefinition> _contractDef)
{
	Json::Value methods(Json::arrayValue);

	for (FunctionDefinition const* f: _contractDef->getInterfaceFunctions())
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

		method["name"] = f->getName();
		method["inputs"] = populateParameters(f->getParameters());
		method["outputs"] = populateParameters(f->getReturnParameters());
		methods.append(method);
	}
	return std::unique_ptr<std::string>(new std::string(m_writer.write(methods)));
}

std::unique_ptr<std::string> InterfaceHandler::getUserDocumentation(std::shared_ptr<ContractDefinition> _contractDef)
{
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	for (FunctionDefinition const* f: _contractDef->getInterfaceFunctions())
	{
		Json::Value user;
		auto strPtr = f->getDocumentation();
		if (strPtr)
		{
			resetUser();
			parseDocString(*strPtr);
			if (!m_notice.empty())
			{// since @notice is the only user tag if missing function should not appear
				user["notice"] = Json::Value(m_notice);
				methods[f->getName()] = user;
			}
		}
	}
	doc["methods"] = methods;

	return std::unique_ptr<std::string>(new std::string(m_writer.write(doc)));
}

std::unique_ptr<std::string> InterfaceHandler::getDevDocumentation(std::shared_ptr<ContractDefinition> _contractDef)
{
	// LTODO: Somewhere in this function warnings for mismatch of param names
	// should be thrown
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	for (FunctionDefinition const* f: _contractDef->getInterfaceFunctions())
	{
		Json::Value method;
		auto strPtr = f->getDocumentation();
		if (strPtr)
		{
			resetDev();
			parseDocString(*strPtr);

			if (!m_dev.empty())
				method["details"] = Json::Value(m_dev);
			Json::Value params(Json::objectValue);
			for (auto const& pair: m_params)
				params[pair.first] = pair.second;

			if (!m_params.empty())
				method["params"] = params;
			if (!m_return.empty())
				method["return"] = m_return;

			if (!method.empty()) // add the function, only if we have any documentation to add
				methods[f->getName()] = method;
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
	m_return.clear();
	m_params.clear();
}

std::string::const_iterator skipLineOrEOS(std::string::const_iterator _nlPos,
										  std::string::const_iterator _end)
{
	return (_nlPos == _end) ? _end : ++_nlPos;
}

std::string::const_iterator InterfaceHandler::parseDocTagLine(std::string::const_iterator _pos,
															  std::string::const_iterator _end,
															  std::string& _tagString,
															  enum DocTagType _tagType)
{
	auto nlPos = std::find(_pos, _end, '\n');
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

	m_lastTag = DOCTAG_PARAM;
	return skipLineOrEOS(nlPos, _end);
}

std::string::const_iterator InterfaceHandler::appendDocTagParam(std::string::const_iterator _pos,
																std::string::const_iterator _end)
{
	// Should never be called with an empty vector
	if (asserts(!m_params.empty()))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Internal: Tried to append to empty parameter"));

	auto pair = m_params.back();
	pair.second += " ";
	auto nlPos = std::find(_pos, _end, '\n');
	std::copy(_pos, nlPos, back_inserter(pair.second));

	m_params.at(m_params.size() - 1) = pair;

	return skipLineOrEOS(nlPos, _end);
}

std::string::const_iterator InterfaceHandler::parseDocTag(std::string::const_iterator _pos,
														  std::string::const_iterator _end,
														  std::string const& _tag)
{
	// LTODO: need to check for @(start of a tag) between here and the end of line
	//      for all cases
	if (m_lastTag == DOCTAG_NONE || _tag != "")
	{
		if (_tag == "dev")
			return parseDocTagLine(_pos, _end, m_dev, DOCTAG_DEV);
		else if (_tag == "notice")
			return parseDocTagLine(_pos, _end, m_notice, DOCTAG_NOTICE);
		else if (_tag == "return")
			return parseDocTagLine(_pos, _end, m_return, DOCTAG_RETURN);
		else if (_tag == "param")
			return parseDocTagParam(_pos, _end);
		else
		{
			// LTODO: Unknown tag, throw some form of warning and not just an exception
			BOOST_THROW_EXCEPTION(DocstringParsingError() << errinfo_comment("Unknown tag " + _tag + " encountered"));
		}
	}
	else
		return appendDocTag(_pos, _end);
}

std::string::const_iterator InterfaceHandler::appendDocTag(std::string::const_iterator _pos,
														   std::string::const_iterator _end)
{
	switch (m_lastTag)
	{
		case DOCTAG_DEV:
			m_dev += " ";
			return parseDocTagLine(_pos, _end, m_dev, DOCTAG_DEV);
		case DOCTAG_NOTICE:
			m_notice += " ";
			return parseDocTagLine(_pos, _end, m_notice, DOCTAG_NOTICE);
		case DOCTAG_RETURN:
			m_return += " ";
			return parseDocTagLine(_pos, _end, m_return, DOCTAG_RETURN);
		case DOCTAG_PARAM:
			return appendDocTagParam(_pos, _end);
		default:
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Internal: Illegal documentation tag type"));
			break;
	}
}

void InterfaceHandler::parseDocString(std::string const& _string)
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
			auto tagNameEndPos = std::find(tagPos, end, ' ');
			if (tagNameEndPos == end)
				BOOST_THROW_EXCEPTION(DocstringParsingError() <<
									  errinfo_comment("End of tag " + std::string(tagPos, tagNameEndPos) + "not found"));

			currPos = parseDocTag(tagNameEndPos + 1, end, std::string(tagPos + 1, tagNameEndPos));
		}
		else if (m_lastTag != DOCTAG_NONE) // continuation of the previous tag
			currPos = appendDocTag(currPos + 1, end);
		else if (currPos != end) // skip the line if a newline was found
			currPos = nlPos + 1;
	}
}

} //solidity NS
} // dev NS
