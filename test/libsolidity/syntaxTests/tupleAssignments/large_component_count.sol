contract C {
	function g() public pure returns (
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint
	) { }
	function f() public pure returns (uint, uint, bytes32) {
		uint a;
		uint b;
		(,,,,a,,,,b,,,,) = g();
	}
}
// ----
// Warning 6321: (194-198): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (200-204): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (206-213): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
