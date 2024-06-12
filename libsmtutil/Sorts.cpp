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

#include <libsmtutil/Sorts.h>

namespace solidity::smtutil
{

Sort* const SortProvider::boolSort{new Sort(Kind::Bool)};
IntSort* const SortProvider::uintSort{new IntSort(false)};
IntSort* const SortProvider::sintSort{new IntSort(true)};
BitVectorSort* const SortProvider::bitVectorSort{new BitVectorSort(256)};

IntSort* SortProvider::intSort(bool _signed)
{
    if (_signed)
        return sintSort;
    return uintSort;
}

}

// Ensure to properly manage memory in the client code using these pointers:

// For example:

int main()
{
    // Use the instances
    solidity::smtutil::Sort* boolSort = solidity::smtutil::SortProvider::boolSort;
    solidity::smtutil::IntSort* intSort = solidity::smtutil::SortProvider::intSort(true);
    solidity::smtutil::BitVectorSort* bitVectorSort = solidity::smtutil::SortProvider::bitVectorSort;

    // Perform operations...

    // Clean up if necessary (for static instances, typically you don't delete them)
    // delete boolSort; // Uncomment only if the lifetime management requires it
    // delete intSort;  // Uncomment only if created with new
    // delete bitVectorSort; // Uncomment only if created with new

    return 0;
}
