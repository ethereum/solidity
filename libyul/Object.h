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
 * Yul code and data object container.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulString.h>

#include <libsolutil/Common.h>

#include <memory>
#include <set>

namespace solidity::yul
{
struct Dialect;
struct AsmAnalysisInfo;


/**
 * Generic base class for both Yul objects and Yul data.
 */
struct ObjectNode
{
	virtual ~ObjectNode() = default;
	virtual std::string toString(Dialect const* _dialect) const = 0;
	std::string toString() { return toString(nullptr); }

	/// Name of the object.
	/// Can be empty since .yul files can also just contain code, without explicitly placing it in an object.
	YulString name;
};

/**
 * Named data in Yul objects.
 */
struct Data: ObjectNode
{
	Data(YulString _name, bytes _data): data(std::move(_data)) { name = _name; }
	std::string toString(Dialect const* _dialect) const override;

	bytes data;
};

/**
 * Yul code and data object container.
 */
struct Object: ObjectNode
{
public:
	/// @returns a (parseable) string representation. Includes types if @a _yul is set.
	std::string toString(Dialect const* _dialect) const override;

	/// @returns the set of names of data objects accessible from within the code of
	/// this object, including the name of object itself
	std::set<YulString> qualifiedDataNames() const;

	/// @returns vector of subIDs if possible to reach subobject with @a _qualifiedName, throws otherwise
	/// For "B.C" should return vector of two values if success (subId of B and subId of C in B).
	/// In object "A" if called for "A.B" will return only one value (subId for B)
	/// will return empty vector for @a _qualifiedName that equals to object name.
	/// Example:
	/// A1{ B2{ C3, D3 }, E2{ F3{ G4, K4, H4{ I5 } } } }
	/// pathToSubObject("A1.E2.F3.H4") == {1, 0, 2}
	/// pathToSubObject("E2.F3.H4") == {1, 0, 2}
	/// pathToSubObject("A1.E2") == {1}
	/// The path must not lead to a @a Data object (will throw in that case).
	std::vector<size_t> pathToSubObject(YulString _qualifiedName) const;

	/// sub id for object if it is subobject of another object, max value if it is not subobject
	size_t subId = std::numeric_limits<size_t>::max();

	std::shared_ptr<Block> code;
	std::vector<std::shared_ptr<ObjectNode>> subObjects;
	std::map<YulString, size_t> subIndexByName;
	std::shared_ptr<yul::AsmAnalysisInfo> analysisInfo;
};

}
