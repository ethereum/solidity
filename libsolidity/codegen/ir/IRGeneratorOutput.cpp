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

#include <libsolidity/codegen/ir/IRGeneratorOutput.h>

#include <libyul/Utilities.h>

#include <libsolutil/StringUtils.h>
#include <libsolutil/Whiskers.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <range/v3/algorithm/none_of.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity::frontend;
using namespace solidity::util;
using namespace solidity::yul;

bool IRGeneratorOutput::isValid() const
{
	static auto const isNull = [](ContractDefinition const* _contract) -> bool { return !_contract; };

	return
		!creation.name.empty() &&
		!deployed.name.empty() &&
		boost::starts_with(creation.code, "{") && boost::ends_with(creation.code, "}") &&
		boost::starts_with(deployed.code, "{") && boost::ends_with(deployed.code, "}") &&
		creation.debugData &&
		deployed.debugData &&
		ranges::none_of(creation.dependencies, isNull) &&
		ranges::none_of(deployed.dependencies, isNull);
}

std::string IRGeneratorOutput::toPrettyString(DependencyResolver const& _dependencyResolver) const
{
	return yul::reindent(toString(_dependencyResolver));
}

std::string IRGeneratorOutput::toString(DependencyResolver const& _dependencyResolver) const
{
	solAssert(isValid());

	auto const recurse = [&](IRGeneratorOutput const& _output) {
		return _output.toString(_dependencyResolver);
	};

	Whiskers t(R"(
		<creationUseSrcComment>
		object "<creationName>" {
			code <creationCode>
			<deployedUseSrcComment>
			object "<deployedName>" {
				code <deployedCode>
				<deployedSubObjects>
				<?hasMetadata>data "<metadataName>" hex"<cborMetadata>"</hasMetadata>
			}
			<creationSubObjects>
		}
	)");

	t("creationName", creation.name);
	t("creationCode", creation.code);
	t("creationUseSrcComment", boost::trim_copy(creation.debugData->formatUseSrcComment()));
	t("creationSubObjects", joinHumanReadable(
		creation.dependencies |
		ranges::views::transform(_dependencyResolver) |
		ranges::views::transform(recurse),
		"" // _separator
	));

	t("deployedName", deployed.name);
	t("deployedCode", deployed.code);
	t("deployedUseSrcComment", boost::trim_copy(deployed.debugData->formatUseSrcComment()));
	t("deployedSubObjects", joinHumanReadable(
		deployed.dependencies |
		ranges::views::transform(_dependencyResolver) |
		ranges::views::transform(recurse),
		"" // _separator
	));
	t("hasMetadata", deployed.metadata != nullptr);
	if (deployed.metadata)
	{
		t("metadataName", deployed.metadata->name.str());
		t("cborMetadata", util::toHex(deployed.metadata->data));
	}

	return t.render();
}
