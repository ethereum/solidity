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

#include <string>

namespace solidity::frontend::test
{

#define NEW_ENCODER(CODE) \
{ \
	string sourceCodeTmp = sourceCode; \
	sourceCode = "pragma abicoder v2;\n" + sourceCode; \
	{ CODE } \
	sourceCode = sourceCodeTmp; \
}

#define OLD_ENCODER(CODE) \
{ \
	string sourceCodeTmp = sourceCode; \
	sourceCode = "pragma abicoder v1;\n" + sourceCode; \
	{ CODE } \
	sourceCode = sourceCodeTmp; \
}

#define BOTH_ENCODERS(CODE) \
{ \
	OLD_ENCODER(CODE) \
	NEW_ENCODER(CODE) \
}

} // end namespaces
