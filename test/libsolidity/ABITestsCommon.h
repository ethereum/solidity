// SPDX-License-Identifier: GPL-3.0

#include <string>

namespace solidity::frontend::test
{

static std::string const NewEncoderPragma = "pragma experimental ABIEncoderV2;\n";

#define NEW_ENCODER(CODE) \
{ \
	sourceCode = NewEncoderPragma + sourceCode; \
	{ CODE } \
}

#define BOTH_ENCODERS(CODE) \
{ \
	{ CODE } \
	NEW_ENCODER(CODE) \
}

} // end namespaces
