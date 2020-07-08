// SPDX-License-Identifier: GPL-3.0
/**
 * @author Liana <liana@ethdev.com>
 * @date 2015
 * Solidity exception hierarchy.
 */

#include <liblangutil/Exceptions.h>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;

Error::Error(ErrorId _errorId, Type _type, SourceLocation const& _location, string const& _description):
	m_errorId(_errorId),
	m_type(_type)
{
	switch (m_type)
	{
	case Type::DeclarationError:
		m_typeName = "DeclarationError";
		break;
	case Type::DocstringParsingError:
		m_typeName = "DocstringParsingError";
		break;
	case Type::ParserError:
		m_typeName = "ParserError";
		break;
	case Type::SyntaxError:
		m_typeName = "SyntaxError";
		break;
	case Type::TypeError:
		m_typeName = "TypeError";
		break;
	case Type::Warning:
		m_typeName = "Warning";
		break;
	}

	if (_location.isValid())
		*this << errinfo_sourceLocation(_location);
	if (!_description.empty())
		*this << util::errinfo_comment(_description);
}

Error::Error(ErrorId _errorId, Error::Type _type, std::string const& _description, SourceLocation const& _location):
	Error(_errorId, _type)
{
	if (_location.isValid())
		*this << errinfo_sourceLocation(_location);
	*this << util::errinfo_comment(_description);
}
