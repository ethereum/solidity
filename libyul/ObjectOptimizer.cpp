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

#include <libyul/ObjectOptimizer.h>

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/EVMMetrics.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/Suite.h>

#include <liblangutil/DebugInfoSelection.h>

#include <libsolutil/Keccak256.h>

#include <boost/algorithm/string.hpp>

#include <limits>
#include <numeric>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;


Dialect const& yul::languageToDialect(Language _language, EVMVersion _version, std::optional<uint8_t> _eofVersion)
{
	switch (_language)
	{
	case Language::Assembly:
	case Language::StrictAssembly:
		return EVMDialect::strictAssemblyForEVMObjects(_version, _eofVersion);
	}
	util::unreachable();
}

void ObjectOptimizer::optimize(Object& _object, Settings const& _settings)
{
	yulAssert(_object.subId == std::numeric_limits<size_t>::max(), "Not a top-level object.");

	optimize(_object, _settings, true /* _isCreation */);
}

void ObjectOptimizer::optimize(Object& _object, Settings const& _settings, bool _isCreation)
{
	yulAssert(_object.code());
	yulAssert(_object.debugData);

	for (auto& subNode: _object.subObjects)
		if (auto subObject = dynamic_cast<Object*>(subNode.get()))
		{
			bool isCreation = !boost::ends_with(subObject->name, "_deployed");
			optimize(
				*subObject,
				_settings,
				isCreation
			);
		}

	Dialect const& dialect = languageToDialect(_settings.language, _settings.evmVersion, _settings.eofVersion);
	std::unique_ptr<GasMeter> meter;
	if (EVMDialect const* evmDialect = dynamic_cast<EVMDialect const*>(&dialect))
		meter = std::make_unique<GasMeter>(*evmDialect, _isCreation, _settings.expectedExecutionsPerDeployment);

	std::optional<h256> cacheKey = calculateCacheKey(_object.code()->root(), *_object.debugData, _settings, _isCreation);
	if (cacheKey.has_value() && m_cachedObjects.count(*cacheKey) != 0)
	{
		overwriteWithOptimizedObject(*cacheKey, _object);
		return;
	}

	OptimiserSuite::run(
		dialect,
		meter.get(),
		_object,
		_settings.optimizeStackAllocation,
		_settings.yulOptimiserSteps,
		_settings.yulOptimiserCleanupSteps,
		_isCreation ? std::nullopt : std::make_optional(_settings.expectedExecutionsPerDeployment),
		{}
	);

	if (cacheKey.has_value())
		storeOptimizedObject(*cacheKey, _object, dialect);
}

void ObjectOptimizer::storeOptimizedObject(util::h256 _cacheKey, Object const& _optimizedObject, Dialect const& _dialect)
{
	m_cachedObjects[_cacheKey] = CachedObject{
		std::make_shared<Block>(ASTCopier{}.translate(_optimizedObject.code()->root())),
		&_dialect,
	};
}

void ObjectOptimizer::overwriteWithOptimizedObject(util::h256 _cacheKey, Object& _object) const
{
	yulAssert(m_cachedObjects.count(_cacheKey) != 0);
	CachedObject const& cachedObject = m_cachedObjects.at(_cacheKey);

	yulAssert(cachedObject.optimizedAST);
	_object.setCode(std::make_shared<AST>(ASTCopier{}.translate(*cachedObject.optimizedAST)));
	yulAssert(_object.code());

	// There's no point in caching AnalysisInfo because it references AST nodes. It can't be shared
	// by multiple ASTs and it's easier to recalculate it than properly clone it.
	yulAssert(cachedObject.dialect);
	_object.analysisInfo = std::make_shared<AsmAnalysisInfo>(
		AsmAnalyzer::analyzeStrictAssertCorrect(
			*cachedObject.dialect,
			_object
		)
	);

	// NOTE: Source name index is included in the key so it must be identical. No need to store and restore it.
}

std::optional<h256> ObjectOptimizer::calculateCacheKey(
	Block const& _ast,
	ObjectDebugData const& _debugData,
	Settings const& _settings,
	bool _isCreation
)
{
	AsmPrinter asmPrinter(
		languageToDialect(_settings.language, _settings.evmVersion, _settings.eofVersion),
		_debugData.sourceNames,
		DebugInfoSelection::All()
	);

	bytes rawKey;
	// NOTE: AsmPrinter never prints nativeLocations included in debug data, so ASTs differing only
	// in that regard are considered equal here.  This is fine because the optimizer does not keep
	// them up to date across AST transformations anyway so in any use where they need to be reliable,
	// we just regenerate them by reparsing the object.
	rawKey += keccak256(asmPrinter(_ast)).asBytes();
	rawKey += keccak256(_debugData.formatUseSrcComment()).asBytes();
	rawKey += h256(u256(_settings.language)).asBytes();
	rawKey += FixedHash<1>(uint8_t(_settings.optimizeStackAllocation ? 0 : 1)).asBytes();
	rawKey += h256(u256(_settings.expectedExecutionsPerDeployment)).asBytes();
	rawKey += FixedHash<1>(uint8_t(_isCreation ? 0 : 1)).asBytes();
	rawKey += keccak256(_settings.evmVersion.name()).asBytes();
	yulAssert(!_settings.eofVersion.has_value() || *_settings.eofVersion > 0);
	rawKey += FixedHash<1>(uint8_t(_settings.eofVersion ? 0 : *_settings.eofVersion)).asBytes();
	rawKey += keccak256(_settings.yulOptimiserSteps).asBytes();
	rawKey += keccak256(_settings.yulOptimiserCleanupSteps).asBytes();

	return h256(keccak256(rawKey));
}
