pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint x = uint(~1);
		// This assertion fails because type conversion is still unsupported.
		assert(x == 2**256 - 2);
		assert(~1 == -2);
	}
}
// ----
// Warning 6328: (169-192): Assertion violation happens here.
