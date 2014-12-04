
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

	BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Internal error"));
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

size_t skipLineOrEOS(std::string const& _string, size_t _nlPos)
{
	return (_nlPos == std::string::npos) ? _string.length() : _nlPos + 1;
}

size_t InterfaceHandler::parseDocTagLine(std::string const& _string,
										 std::string& _tagString,
										 size_t _pos,
										 enum DocTagType _tagType)
{
	size_t nlPos = _string.find('\n', _pos);
	_tagString += _string.substr(_pos,
								 nlPos == std::string::npos ?
								 _string.length() - _pos:
								 nlPos - _pos);
	m_lastTag = _tagType;
	return skipLineOrEOS(_string, nlPos);
}

size_t InterfaceHandler::parseDocTagParam(std::string const& _string, size_t _startPos)
{
	// find param name
	size_t currPos = _string.find(' ', _startPos);
	if (currPos == std::string::npos)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("End of param name not found"));


	auto paramName = _string.substr(_startPos, currPos - _startPos);

	currPos += 1;
	size_t nlPos = _string.find('\n', currPos);
	auto paramDesc = _string.substr(currPos,
									nlPos == std::string::npos ?
									_string.length() - currPos :
									nlPos - currPos);

	m_params.push_back(std::make_pair(paramName, paramDesc));

	m_lastTag = DOCTAG_PARAM;
	return skipLineOrEOS(_string, nlPos);
}

size_t InterfaceHandler::appendDocTagParam(std::string const& _string, size_t _startPos)
{
	// Should never be called with an empty vector
	if (asserts(!m_params.empty()))
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Internal: Tried to append to empty parameter"));
	auto pair = m_params.back();
	size_t nlPos = _string.find('\n', _startPos);
	pair.second += _string.substr(_startPos,
								 nlPos == std::string::npos ?
								 _string.length() - _startPos :
								 nlPos - _startPos);

	m_params.at(m_params.size() - 1) = pair;

	return skipLineOrEOS(_string, nlPos);
}

size_t InterfaceHandler::parseDocTag(std::string const& _string, std::string const& _tag, size_t _pos)
{
	// LTODO: need to check for @(start of a tag) between here and the end of line
	//      for all cases
	size_t nlPos = _pos;
	if (m_lastTag == DOCTAG_NONE || _tag != "")
	{
		if (_tag == "dev")
			nlPos = parseDocTagLine(_string, m_dev, _pos, DOCTAG_DEV);
		else if (_tag == "notice")
			nlPos = parseDocTagLine(_string, m_notice, _pos, DOCTAG_NOTICE);
		else if (_tag == "return")
			nlPos = parseDocTagLine(_string, m_return, _pos, DOCTAG_RETURN);
		else if (_tag == "param")
			nlPos = parseDocTagParam(_string, _pos);
		else
		{
			// LTODO: Unknown tas, throw some form of warning
		}
	}
	else
		appendDocTag(_string, _pos);

	return nlPos;
}

size_t InterfaceHandler::appendDocTag(std::string const& _string, size_t _startPos)
{
	size_t newPos = _startPos;
	switch (m_lastTag)
	{
		case DOCTAG_DEV:
			m_dev += " ";
			newPos = parseDocTagLine(_string, m_dev, _startPos, DOCTAG_DEV);
			break;
		case DOCTAG_NOTICE:
			m_notice += " ";
			newPos = parseDocTagLine(_string, m_notice, _startPos, DOCTAG_NOTICE);
			break;
		case DOCTAG_RETURN:
			m_return += " ";
			newPos = parseDocTagLine(_string, m_return, _startPos, DOCTAG_RETURN);
			break;
		case DOCTAG_PARAM:
			newPos = appendDocTagParam(_string, _startPos);
			break;
		default:
			break;
		}
	return newPos;
}

void InterfaceHandler::parseDocString(std::string const& _string, size_t _startPos)
{
	size_t pos2;
	size_t newPos = _startPos;
	size_t tagPos = _string.find('@', _startPos);
	size_t nlPos = _string.find('\n', _startPos);

	if (tagPos != std::string::npos && tagPos < nlPos)
	{
		// we found a tag
		pos2 = _string.find(' ', tagPos);
		if (pos2 == std::string::npos)
			BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("End of tag not found"));

		newPos = parseDocTag(_string, _string.substr(tagPos + 1, pos2 - tagPos - 1), pos2 + 1);
	}
	else if (m_lastTag != DOCTAG_NONE) // continuation of the previous tag
		newPos = appendDocTag(_string, _startPos + 1);
	else // skip the line if a newline was found
	{
		if (newPos != std::string::npos)
			newPos = nlPos + 1;
	}
	if (newPos == std::string::npos || newPos == _string.length())
		return; // EOS
	parseDocString(_string, newPos);
}

} //solidity NS
} // dev NS
