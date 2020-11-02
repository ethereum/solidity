pragma experimental SMTChecker;

contract C {
	function g() public pure returns (uint, uint) {
		uint a;
		uint b;
		(a, b) = g();
	}
}
//
// ----
// Warning 6321: (81-85): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (87-91): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
