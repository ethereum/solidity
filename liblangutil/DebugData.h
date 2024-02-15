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

#include <liblangutil/SourceLocation.h>
#include <libsolutil/JSON.h>
#include <optional>
#include <memory>

namespace solidity::langutil
{

struct DebugData
{
	typedef typename std::shared_ptr<DebugData const> ConstPtr;
	typedef std::optional<std::vector<std::shared_ptr<Json>>> Attributes;

	explicit DebugData(
		langutil::SourceLocation _nativeLocation = {},
		langutil::SourceLocation _originLocation = {},
		std::optional<int64_t> _astID = {},
		Attributes _attributes = {}
	):
		nativeLocation(std::move(_nativeLocation)),
		originLocation(std::move(_originLocation)),
		astID(_astID),
		attributes(std::move(_attributes))
	{}

	static DebugData::ConstPtr create(
		langutil::SourceLocation _nativeLocation,
		langutil::SourceLocation _originLocation = {},
		std::optional<int64_t> _astID = {},
		Attributes _attributes = {}
	)
	{
		return std::make_shared<DebugData>(
			std::move(_nativeLocation),
			std::move(_originLocation),
			_astID,
			std::move(_attributes)
		);
	}

	static DebugData::ConstPtr create()
	{
		static DebugData::ConstPtr emptyDebugData = create({});
		return emptyDebugData;
	}

	/// Location in the Yul code.
	langutil::SourceLocation nativeLocation;
	/// Location in the original source that the Yul code was produced from.
	/// Optional. Only present if the Yul source contains location annotations.
	langutil::SourceLocation originLocation;
	/// ID in the (Solidity) source AST.
	std::optional<int64_t> astID;
	/// Additional debug data attributes.
	Attributes attributes;
};

} // namespace solidity::langutil
