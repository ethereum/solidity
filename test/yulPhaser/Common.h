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
 * Miscellaneous utilities for use in yul-phaser's test cases.
 *
 * - Generic code that's only used in these particular tests.
 * - Convenience functions and wrappers to make tests more concise.
 * - Mocks and dummy objects/functions used in multiple test suites.
 *
 * Note that the code included here may be not as generic, robust and/or complete as it could be
 * since it's not meant for production use. If some utility seems useful enough to be moved to
 * the normal code base, you should review its implementation before doing so.
 */

#pragma once
