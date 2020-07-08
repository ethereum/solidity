// SPDX-License-Identifier: GPL-3.0


#include <libsmtutil/Sorts.h>

using namespace std;

namespace solidity::smtutil
{

shared_ptr<Sort> const SortProvider::boolSort{make_shared<Sort>(Kind::Bool)};
shared_ptr<IntSort> const SortProvider::uintSort{make_shared<IntSort>(false)};
shared_ptr<IntSort> const SortProvider::sintSort{make_shared<IntSort>(true)};

shared_ptr<IntSort> SortProvider::intSort(bool _signed)
{
	if (_signed)
		return sintSort;
	return uintSort;
}

}
