pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint x = uint(~1);
		assert(x == 2**256 - 2);
		assert(~1 == -2);
	}
}
// ----
// Warning 6328: (169-192): CHC: Assertion violation happens here.
