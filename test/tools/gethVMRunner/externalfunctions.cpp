#include "externalfunctions.h"

extern "C" {
#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG)                                  \
  __attribute__((weak)) RETURN_TYPE NAME FUNC_SIG

#include "externalfunctions.def"

#undef EXT_FUNC
}

#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG)                                  \
  __attribute__((weak)) RETURN_TYPE NAME FUNC_SIG

#undef EXT_FUNC

ExternalFunctions::ExternalFunctions()
{
#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG)                                  \
  this->NAME = ::NAME;                                                         \


#include "externalfunctions.def"

#undef EXT_FUNC
}
