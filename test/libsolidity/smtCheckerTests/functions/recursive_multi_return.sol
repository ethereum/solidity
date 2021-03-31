contract C {
	function g() public pure returns (uint, uint) {
		uint a;
		uint b;
		(a, b) = g();
	}
}
//
// ====
// SMTEngine: all
// ----
// Warning 6321: (48-52): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (54-58): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
