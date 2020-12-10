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

#include <libsmtutil/Z3Loader.h>
#include <libsmtutil/Exceptions.h>
#include <z3.h>
#include <z3_version.h>
#include <vector>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

using namespace std;
using namespace solidity;
using namespace solidity::smtutil;

Z3Loader const& Z3Loader::get()
{
	static Z3Loader z3;
	return z3;
}

void* Z3Loader::loadSymbol(char const* _name) const
{
	smtAssert(m_handle, "Attempted to use dynamically loaded Z3, even though it is not available.");
	void* sym = dlsym(m_handle, _name);
	smtAssert(sym, string("Symbol \"") + _name + "\" not found in libz3.so");
	return sym;
}

bool Z3Loader::available() const
{
	if (m_handle == nullptr)
		return false;
	unsigned major = 0;
	unsigned minor = 0;
	unsigned build = 0;
	unsigned rev = 0;
	Z3_get_version(&major, &minor, &build, &rev);
	return major == Z3_MAJOR_VERSION && minor == Z3_MINOR_VERSION;
}

Z3Loader::Z3Loader()
{
	string libname{"libz3.so." + to_string(Z3_MAJOR_VERSION) + "." + to_string(Z3_MINOR_VERSION)};
	m_handle = dlmopen(LM_ID_NEWLM, libname.c_str(), RTLD_NOW);
}

Z3Loader::~Z3Loader()
{
	if (m_handle)
		dlclose(m_handle);
}
