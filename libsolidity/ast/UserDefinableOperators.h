#pragma once

#include <liblangutil/Token.h>

#include <vector>

namespace solidity::frontend
{

std::vector<langutil::Token> const userDefinableOperators = {
	// Bitwise
	langutil::Token::BitOr,
	langutil::Token::BitAnd,
	langutil::Token::BitXor,
	langutil::Token::BitNot,
	// Arithmetic
	langutil::Token::Add,
	langutil::Token::Sub,
	langutil::Token::Mul,
	langutil::Token::Div,
	langutil::Token::Mod,
	// Comparison
	langutil::Token::Equal,
	langutil::Token::NotEqual,
	langutil::Token::LessThan,
	langutil::Token::GreaterThan,
	langutil::Token::LessThanOrEqual,
	langutil::Token::GreaterThanOrEqual,
};

}
