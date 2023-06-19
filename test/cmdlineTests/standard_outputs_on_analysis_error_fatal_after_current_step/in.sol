// SPDX-License-Identifier: GPL-3.0
pragma solidity *;

// This will trigger a fatal error at the analysis stage, of the kind that lets the current
// analysis steps finish but terminates analysis after immediately after that step.
function f(uint immutable x) {}
