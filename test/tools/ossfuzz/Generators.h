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
/**
 * Convenience macros for Solidity generator type declarations.
 */

#pragma once

/*
 * Alphabetically sorted Generator types.
 * SEP must appear between two elements and ENDSEP must
 * appear after the last element.
 *
 * This macro applies another macro (MACRO) that is
 * passed as input to this macro on the list of Generator
 * types. Example that uses forward declaration:
 *
 * #define MACRO(G) class G;
 * #define SEMICOLON() ;
 *
 * GENERATORLIST(MACRO, SEMICOLON(), SEMICOLON())
 *
 * produces
 *
 * class PragmaGenerator;class SourceUnitGenerator;class TestCaseGenerator;
 *
 */
#define GENERATORLIST(MACRO, SEP, ENDSEP) \
	MACRO(ImportGenerator) SEP \
	MACRO(PragmaGenerator) SEP \
	MACRO(SourceUnitGenerator) SEP \
	MACRO(TestCaseGenerator) ENDSEP
