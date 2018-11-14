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
/**
 * @date 2017
 * Common functions the Yul tests.
 */

#pragma once

#include <libsolidity/inlineasm/AsmData.h>

#include <string>
#include <vector>
#include <memory>

namespace langutil
{
class Scanner;
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;
}

namespace dev
{
namespace solidity
{
namespace assembly
{
struct AsmAnalysisInfo;
}
}
namespace yul
{
namespace test
{

void printErrors(langutil::ErrorList const& _errors, langutil::Scanner const& _scanner);
std::pair<std::shared_ptr<solidity::assembly::Block>, std::shared_ptr<solidity::assembly::AsmAnalysisInfo>>
parse(std::string const& _source, bool _yul = true);
solidity::assembly::Block disambiguate(std::string const& _source, bool _yul = true);
std::string format(std::string const& _source, bool _yul = true);

}
}
}
