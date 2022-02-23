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

#pragma once

namespace solidity::smtutil
{

class Z3Loader
{
public:
	Z3Loader(Z3Loader const&) = delete;
	Z3Loader& operator=(Z3Loader const&) = delete;
	static Z3Loader const& get();
	void* loadSymbol(char const* _name) const;
	bool available() const;
private:
	Z3Loader();
	~Z3Loader();
	void* m_handle = nullptr;
};

}