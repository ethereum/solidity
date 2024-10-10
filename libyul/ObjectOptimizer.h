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

#include <libyul/ASTForward.h>
#include <libyul/Object.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/FixedHash.h>

#include <map>
#include <memory>
#include <optional>

namespace solidity::yul
{

enum class Language
{
	Assembly,
	StrictAssembly,
};

Dialect const& languageToDialect(Language _language, langutil::EVMVersion _version, std::optional<uint8_t> _eofVersion);

/// Encapsulates logic for applying @a yul::OptimiserSuite to a whole hierarchy of Yul objects.
/// Also, acts as a transparent cache for optimized objects.
///
/// The cache is designed to allow sharing its instances widely across the compiler, without the
/// need to invalidate entries due to changing settings or context.
/// Caching is performed at the granularity of individual ASTs rather than whole object trees,
/// which means that reuse is possible even within a single hierarchy, e.g. when creation and
/// deployed objects have common dependencies.
class ObjectOptimizer
{
public:
	/// Optimization settings and context information.
	/// This information becomes a part of the cache key and, together with the object content,
	/// must uniquely determine the result of optimization.
	struct Settings
	{
		Language language;
		langutil::EVMVersion evmVersion;
		std::optional<uint8_t> eofVersion;
		bool optimizeStackAllocation;
		std::string yulOptimiserSteps;
		std::string yulOptimiserCleanupSteps;
		size_t expectedExecutionsPerDeployment;
	};

	/// Recursively optimizes a Yul object with given settings, reusing cached ASTs where possible
	/// or caching the result otherwise. The object is modified in-place.
	/// Automatically accounts for the difference between creation and deployed objects.
	/// @warning Does not ensure that nativeLocations in the resulting AST match the optimized code.
	void optimize(Object& _object, Settings const& _settings);

	size_t size() const { return m_cachedObjects.size(); }

private:
	struct CachedObject
	{
		std::shared_ptr<Block const> optimizedAST;
		Dialect const* dialect;
	};

	void optimize(Object& _object, Settings const& _settings, bool _isCreation);

	void storeOptimizedObject(util::h256 _cacheKey, Object const& _optimizedObject, Dialect const& _dialect);
	void overwriteWithOptimizedObject(util::h256 _cacheKey, Object& _object) const;

	static std::optional<util::h256> calculateCacheKey(
		Block const& _ast,
		ObjectDebugData const& _debugData,
		Settings const& _settings,
		bool _isCreation
	);

	std::map<util::h256, CachedObject> m_cachedObjects;
};

}
