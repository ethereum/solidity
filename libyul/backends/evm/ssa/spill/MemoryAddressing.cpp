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

#include <libyul/backends/evm/ssa/spill/MemoryAddressing.h>

#include <libyul/Exceptions.h>

#include <cstdint>

using namespace solidity::yul::ssa::spill;


MemoryAddressing::MemoryAddressing(ControlFlowGraphs& _cfgs, std::span<SpillSet const> _spillSetsPerCFG):
	m_addresses(_spillSetsPerCFG.size())
{
	yulAssert(_spillSetsPerCFG.size() == _cfgs.functionGraphs.size());

	std::size_t totalSlots = 0;
	for (auto const& spillSet: _spillSetsPerCFG)
		totalSlots += spillSet.numSpilled();

	if (totalSlots == 0)
		return;

	if (!_cfgs.memoryGuard.has_value())
		BOOST_THROW_EXCEPTION(StackTooDeepError(
			YulName{},
			YulName{},
			0,
			"Stack too deep."
			"\nNo memoryguard was present. "
			"Consider using memory-safe assembly only and annotating it via "
			"'assembly (\"memory-safe\") { ... }'."
		));
	u256 const originalGuard = *_cfgs.memoryGuard;
	*_cfgs.memoryGuard += u256(totalSlots) * 32;

	std::uint32_t globalSlot = 0;
	for (std::size_t i = 0; i < _spillSetsPerCFG.size(); ++i)
	{
		auto const& spillSet = _spillSetsPerCFG[i];
		auto& cfgMap = m_addresses[i];
		cfgMap.reserve(spillSet.numSpilled());
		for (SpillKey const key: spillSet.spilledValues())
			cfgMap.emplace(key, originalGuard + u256(32) * globalSlot++);
	}
	yulAssert(globalSlot == totalSlots);
}

solidity::u256 MemoryAddressing::addressOf(FunctionGraphID _cfg, SpillKey const _key) const
{
	yulAssert(_cfg < m_addresses.size(), fmt::format("CFG index out of range: {}", _cfg));
	auto const& cfgMap = m_addresses[_cfg];
	auto const it = cfgMap.find(_key);
	yulAssert(it != cfgMap.end(), fmt::format("not spilled: cfg={} variable={}", _cfg, _key));
	return it->second;
}
bool MemoryAddressing::hasAddress(FunctionGraphID _cfg, SpillKey const _key) const
{
	yulAssert(_cfg < m_addresses.size(), fmt::format("CFG index out of range: {}", _cfg));
	auto const& cfgMap = m_addresses[_cfg];
	return cfgMap.contains(_key);
}
