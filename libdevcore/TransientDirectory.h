/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file TransientDirectory.h
 * @author Marek Kotewicz <marek@ethdev.com>
 * @date 2015
 */

#pragma once

#include <string>

namespace dev
{

/**
 * @brief temporary directory implementation
 * It creates temporary directory in the given path. On dealloc it removes the directory
 * @throws if the given path already exists, throws an exception
 */
class TransientDirectory
{
public:
	TransientDirectory();
	TransientDirectory(std::string const& _path);
	~TransientDirectory();

	std::string const& path() const { return m_path; }

private:
	std::string m_path;
};

}
