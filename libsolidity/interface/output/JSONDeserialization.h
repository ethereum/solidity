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
/// Deserializes the standard-JSON encoding (solc's `--standard-json` interface) into `solidity::frontend::output::Output` and its member types via nlohmann's ADL `from_json` convention.

#pragma once

#include <libsolidity/interface/output/Output.h>

#include <libsolutil/JSON.h>

#include <concepts>
#include <string_view>

namespace solidity::frontend::output
{

/** The concept — mirrors nlohmann's ADL pattern
 *
 * A parser just needs to hand out SubDocuments and convert them to T.
 * The from_json free functions do the actual mapping, found via ADL.template<typename P>
 */
template<typename P>
concept JsonParser =
    requires { typename P::SubDocument; }
    && requires(P p, std::string_view raw) {
        { p.parse(raw) } -> std::same_as<typename P::SubDocument>;
    };

// How the parser extracts T from a SubDocument — calls from_json via ADL
template<typename T, typename P>
    requires JsonParser<P>
T extract(const typename P::SubDocument& sub) {
    T result;
    from_json(sub, result);  // found via ADL, just like nlohmann
    return result;
}

template<typename T, typename P>
    requires JsonParser<P>
T extract(P& parser, std::string_view raw) {
    return extract<T, P>(parser.parse(raw));
}

struct NlohmannParser {
    using SubDocument = nlohmann::json;

    SubDocument parse(std::string_view raw) {
        return nlohmann::json::parse(raw);
    }
};

void from_json(Json const&, SourceLocation&);
void from_json(Json const&, Error&);
void from_json(Json const&, Source&);
void from_json(Json const&, ABIParameter&);
void from_json(Json const&, ABIConstructor&);
void from_json(Json const&, ABIFunction&);
void from_json(Json const&, ABIFallback&);
void from_json(Json const&, ABIReceive&);
void from_json(Json const&, ABIEvent&);
void from_json(Json const&, ABIError&);
void from_json(Json const&, ABIEntry&);
void from_json(Json const&, ByteOffset&);
void from_json(Json const&, Bytecode&);
void from_json(Json const&, EVM&);
void from_json(Json const&, Contract&);
void from_json(Json const&, Contracts&);
void from_json(Json const&, Output&);

}
