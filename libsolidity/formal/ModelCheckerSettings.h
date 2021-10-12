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

#include <libsmtutil/SolverInterface.h>

#include <optional>
#include <set>

namespace solidity::frontend
{

struct ModelCheckerContracts
{
	/// By default all contracts are analyzed.
	static ModelCheckerContracts Default() { return {}; }

	/// Parses a string of the form <path>:<contract>,<path>:contract,...
	/// and returns nullopt if a path or contract name is empty.
	static std::optional<ModelCheckerContracts> fromString(std::string const& _contracts);

	/// @returns true if all contracts should be analyzed.
	bool isDefault() const { return contracts.empty(); }

	bool has(std::string const& _source) const { return contracts.count(_source); }
	bool has(std::string const& _source, std::string const& _contract) const
	{
		return has(_source) && contracts.at(_source).count(_contract);
	}

	bool operator!=(ModelCheckerContracts const& _other) const noexcept { return !(*this == _other); }
	bool operator==(ModelCheckerContracts const& _other) const noexcept { return contracts == _other.contracts; }

	/// Represents which contracts should be analyzed by the SMTChecker
	/// as the most derived.
	/// The key is the source file. If the map is empty, all sources must be analyzed.
	/// For each source, contracts[source] represents the contracts in that source
	/// that should be analyzed.
	/// If the set of contracts is empty, all contracts in that source should be analyzed.
	std::map<std::string, std::set<std::string>> contracts;
};

struct ModelCheckerEngine
{
	bool bmc = false;
	bool chc = false;

	static constexpr ModelCheckerEngine All() { return {true, true}; }
	static constexpr ModelCheckerEngine BMC() { return {true, false}; }
	static constexpr ModelCheckerEngine CHC() { return {false, true}; }
	static constexpr ModelCheckerEngine None() { return {false, false}; }

	bool none() const { return !any(); }
	bool any() const { return bmc || chc; }
	bool all() const { return bmc && chc; }

	static std::optional<ModelCheckerEngine> fromString(std::string const& _engine)
	{
		static std::map<std::string, ModelCheckerEngine> engineMap{
			{"all", All()},
			{"bmc", BMC()},
			{"chc", CHC()},
			{"none", None()}
		};
		if (engineMap.count(_engine))
			return engineMap.at(_engine);
		return {};
	}

	bool operator!=(ModelCheckerEngine const& _other) const noexcept { return !(*this == _other); }
	bool operator==(ModelCheckerEngine const& _other) const noexcept { return bmc == _other.bmc && chc == _other.chc; }
};

enum class InvariantType { Contract, Reentrancy };

struct ModelCheckerInvariants
{
	/// Adds the default targets, that is, all except underflow and overflow.
	static ModelCheckerInvariants Default() { return *fromString("default"); }
	/// Adds all targets, including underflow and overflow.
	static ModelCheckerInvariants All() { return *fromString("all"); }
	static ModelCheckerInvariants None() { return {{}}; }

	static std::optional<ModelCheckerInvariants> fromString(std::string const& _invs);

	bool has(InvariantType _inv) const { return invariants.count(_inv); }

	/// @returns true if the @p _target is valid,
	/// and false otherwise.
	bool setFromString(std::string const& _target);

	static std::map<std::string, InvariantType> const validInvariants;

	bool operator!=(ModelCheckerInvariants const& _other) const noexcept { return !(*this == _other); }
	bool operator==(ModelCheckerInvariants const& _other) const noexcept { return invariants == _other.invariants; }

	std::set<InvariantType> invariants;
};

enum class VerificationTargetType { ConstantCondition, Underflow, Overflow, UnderOverflow, DivByZero, Balance, Assert, PopEmptyArray, OutOfBounds };

struct ModelCheckerTargets
{
	/// Adds the default targets, that is, all except underflow and overflow.
	static ModelCheckerTargets Default() { return *fromString("default"); }
	/// Adds all targets, including underflow and overflow.
	static ModelCheckerTargets All() { return *fromString("all"); }

	static std::optional<ModelCheckerTargets> fromString(std::string const& _targets);

	bool has(VerificationTargetType _type) const { return targets.count(_type); }

	/// @returns true if the @p _target is valid,
	/// and false otherwise.
	bool setFromString(std::string const& _target);

	static std::map<std::string, VerificationTargetType> const targetStrings;

	static std::map<VerificationTargetType, std::string> const targetTypeToString;

	bool operator!=(ModelCheckerTargets const& _other) const noexcept { return !(*this == _other); }
	bool operator==(ModelCheckerTargets const& _other) const noexcept { return targets == _other.targets; }

	std::set<VerificationTargetType> targets;
};

struct ModelCheckerExtCalls
{
	enum class Mode
	{
		UNTRUSTED,
		TRUSTED
	};

	Mode mode = Mode::UNTRUSTED;

	static std::optional<ModelCheckerExtCalls> fromString(std::string const& _mode);

	bool isTrusted() const { return mode == Mode::TRUSTED; }
};

struct ModelCheckerSettings
{
	ModelCheckerContracts contracts = ModelCheckerContracts::Default();
	/// Currently division and modulo are replaced by multiplication with slack vars, such that
	/// a / b <=> a = b * k + m
	/// where k and m are slack variables.
	/// This is the default because Spacer prefers that over precise / and mod.
	/// This option allows disabling this mechanism since other solvers
	/// might prefer the precise encoding.
	bool divModNoSlacks = false;
	ModelCheckerEngine engine = ModelCheckerEngine::None();
	ModelCheckerExtCalls externalCalls = {};
	ModelCheckerInvariants invariants = ModelCheckerInvariants::Default();
	bool showUnproved = false;
	smtutil::SMTSolverChoice solvers = smtutil::SMTSolverChoice::All();
	ModelCheckerTargets targets = ModelCheckerTargets::Default();
	std::optional<unsigned> timeout;

	bool operator!=(ModelCheckerSettings const& _other) const noexcept { return !(*this == _other); }
	bool operator==(ModelCheckerSettings const& _other) const noexcept
	{
		return
			contracts == _other.contracts &&
			divModNoSlacks == _other.divModNoSlacks &&
			engine == _other.engine &&
			externalCalls.mode == _other.externalCalls.mode &&
			invariants == _other.invariants &&
			showUnproved == _other.showUnproved &&
			solvers == _other.solvers &&
			targets == _other.targets &&
			timeout == _other.timeout;
	}
};

}
