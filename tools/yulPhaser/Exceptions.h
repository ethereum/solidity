// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolutil/Exceptions.h>

namespace solidity::phaser
{

struct BadInput: virtual util::Exception {};
struct InvalidProgram: virtual BadInput {};
struct NoInputFiles: virtual BadInput {};
struct MissingFile: virtual BadInput {};

struct FileOpenError: virtual util::Exception {};
struct FileReadError: virtual util::Exception {};
struct FileWriteError: virtual util::Exception {};

}
