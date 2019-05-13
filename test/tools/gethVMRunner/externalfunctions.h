#pragma once

#include <stddef.h>
#include <stdint.h>
#include "go-boilerplate.h"
#include "fuzzer.h"

struct ExternalFunctions
{
	ExternalFunctions();

#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG)                                  \
  RETURN_TYPE(*NAME) FUNC_SIG = nullptr

#include "externalfunctions.def"

#undef EXT_FUNC
};