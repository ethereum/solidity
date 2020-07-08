// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolutil/Assertions.h>
#include <libsolutil/Exceptions.h>

namespace solidity::smtutil
{

struct SMTLogicError: virtual util::Exception {};

#define smtAssert(CONDITION, DESCRIPTION) \
	assertThrow(CONDITION, SMTLogicError, DESCRIPTION)

}
