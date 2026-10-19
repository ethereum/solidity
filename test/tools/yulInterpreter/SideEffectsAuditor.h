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

#include <array>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace solidity::yul
{
class Dialect;
}

namespace solidity::yul::test
{

struct InterpreterState;

enum class Claim
{
	CannotContinue,
	CannotRevert,
	CannotTerminate,
	DoesNotWriteStorage,
	DoesNotAccessStorage,
	DoesNotWriteMemory,
	DoesNotAccessMemory,
	CanBeRemoved,
	Movable
};

std::array<Claim, 9> constexpr allClaims{
	Claim::CannotContinue,
	Claim::CannotRevert,
	Claim::CannotTerminate,
	Claim::DoesNotWriteStorage,
	Claim::DoesNotAccessStorage,
	Claim::DoesNotWriteMemory,
	Claim::DoesNotAccessMemory,
	Claim::CanBeRemoved,
	Claim::Movable
};

std::string_view format_as(Claim _claim);

struct SideEffectsAudit
{
	/// How often each kind of claim was checked
	std::map<Claim, std::size_t> checks;
	/// How many of these checks were of a call that involves a user-defined function, directly or in its arguments
	std::map<Claim, std::size_t> checksInvolvingUserFunctions;
	/// The first claim that did not hold, with the call it was made about
	std::optional<std::string> violation;
};

/// Runs @a _ast in the interpreter and checks what SideEffectsCollector and ControlFlowSideEffectsCollector
/// claim about every function call it evaluates, arguments included:
///   - a call that cannot continue, revert or terminate does not
///   - a call that does not write storage or memory leaves it unchanged, and one without an effect on it does
///     not access it
///   - a call that can be removed changes neither and continues
///   - a movable call accesses neither and evaluates to the same values a second time
/// Claims about transient storage and other state, and that a call cannot loop, are not checked.
/// Requires unique function names and all function definitions in the outermost block.
/// Throws what Interpreter::run throws if the interpreter hits one of its limits.
SideEffectsAudit auditSideEffects(InterpreterState& _state, Dialect const& _dialect, Block const& _ast);

}
