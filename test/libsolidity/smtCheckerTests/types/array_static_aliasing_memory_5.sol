pragma experimental SMTChecker;

contract C
{
	function f(uint[2] memory a, uint[2] memory b, uint[2] memory c) internal pure {
		require(c[0] == 42);
		require(a[0] == 2);
		b[0] = 1;
		// Should fail since b == c is possible.
		assert(c[0] == 42);
		// Should fail since b == a is possible.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (230-248): Assertion violation happens here
// Warning: (295-312): Assertion violation happens here
