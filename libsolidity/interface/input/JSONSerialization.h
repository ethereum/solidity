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
/// Maps `solidity::frontend::input::Input` and its member types to the standard-JSON encoding
/// (solc's `--standard-json` interface) via nlohmann's ADL `to_json` convention.

#pragma once

#include <libsolidity/interface/input/Input.h>

#include <libsolutil/JSON.h>

namespace solidity::frontend::input
{

void to_json(Json& _json, YulOptimizerDetails const& _details);
void to_json(Json& _json, OptimizerDetails const& _details);
void to_json(Json& _json, Optimizer const& _optimizer);
void to_json(Json& _json, Debug const& _debug);
void to_json(Json& _json, Metadata const& _metadata);
void to_json(Json& _json, ModelChecker const& _modelChecker);
void to_json(Json& _json, Settings const& _settings);
void to_json(Json& _json, Source const& _source);
void to_json(Json& _json, Input const& _input);

}