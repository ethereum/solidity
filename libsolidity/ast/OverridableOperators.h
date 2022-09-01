#pragma once

#include <liblangutil/Token.h>

#include <vector>

namespace solidity::frontend
{

std::vector<langutil::Token> const overridableOperators = {
	langutil::Token::BitOr,
	langutil::Token::BitAnd,
	langutil::Token::BitXor,
	langutil::Token::Add,
	langutil::Token::Sub,
	langutil::Token::Mul,
	langutil::Token::Div,
	langutil::Token::Mod,
	langutil::Token::Equal,
	langutil::Token::NotEqual,
	langutil::Token::LessThan,
	langutil::Token::GreaterThan,
	langutil::Token::LessThanOrEqual,
	langutil::Token::GreaterThanOrEqual,
	langutil::Token::BitNot,
	langutil::Token::SHL,
	langutil::Token::SAR,
	langutil::Token::Exp,
	langutil::Token::Not
};

}
