// SPDX-License-Identifier: GPL-3.0
/** @file ErrorCheck.h
 * @author Yoichi Hirai <i@yoichihirai.com>
 * @date 2016
 */

#pragma once

#include <liblangutil/Exceptions.h>

#include <vector>
#include <tuple>

namespace solidity::frontend::test
{

bool searchErrorMessage(langutil::Error const& _err, std::string const& _substr);

/// Checks that all provided errors are of the given type and have a given substring in their
/// description.
/// If the expectations are not met, returns a nonempty description, otherwise an empty string.
std::string searchErrors(langutil::ErrorList const& _errors, std::vector<std::pair<langutil::Error::Type, std::string>> const& _expectations);

}
