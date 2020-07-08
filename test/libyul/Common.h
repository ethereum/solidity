// SPDX-License-Identifier: GPL-3.0
/**
 * @date 2017
 * Common functions the Yul tests.
 */

#pragma once

#include <libyul/AsmData.h>

#include <liblangutil/EVMVersion.h>

#include <string>
#include <vector>
#include <memory>

namespace solidity::langutil
{
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;
}

namespace solidity::yul
{
struct AsmAnalysisInfo;
struct Dialect;
}

namespace solidity::yul::test
{

void printErrors(langutil::ErrorList const& _errors);

std::pair<std::shared_ptr<Block>, std::shared_ptr<AsmAnalysisInfo>>
parse(std::string const& _source, bool _yul = true);

std::pair<std::shared_ptr<Block>, std::shared_ptr<AsmAnalysisInfo>>
parse(std::string const& _source, Dialect const& _dialect, langutil::ErrorList& _errors);

Block disambiguate(std::string const& _source, bool _yul = true);
std::string format(std::string const& _source, bool _yul = true);

solidity::yul::Dialect const& dialect(std::string const& _name, langutil::EVMVersion _evmVersion);

}
