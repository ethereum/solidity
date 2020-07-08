// SPDX-License-Identifier: GPL-3.0
/**
 * Yul code and data object container.
 */

#pragma once

#include <libyul/AsmDataForward.h>
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
	/// this object.
	std::set<YulString> dataNames() const;

	std::shared_ptr<Block> code;
	std::vector<std::shared_ptr<ObjectNode>> subObjects;
	std::map<YulString, size_t> subIndexByName;
	std::shared_ptr<yul::AsmAnalysisInfo> analysisInfo;
};

}
