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
// Warning: (126-129): Assertion checker does not support recursive function calls.
