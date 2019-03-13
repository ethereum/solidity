pragma experimental SMTChecker;

contract C
{
	function f(
		uint[] memory a,
		uint[] memory b,
		uint[][] memory cc,
		uint8[][] memory dd,
		uint[][][] memory eee
	) internal pure {
		require(a[0] == 2);
		require(cc[0][0] == 50);
		require(dd[0][0] == 10);
		require(eee[0][0][0] == 50);
		b[0] = 1;
		// Fails because b == a is possible.
		assert(a[0] == 2);
		// Fails because b == cc[0] is possible.
		assert(cc[0][0] == 50);
		// Should not fail since knowledge is erased only for uint[].
		assert(dd[0][0] == 10);
		// Fails because b == ee[0][0] is possible.
		assert(eee[0][0][0] == 50);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (345-362): Assertion violation happens here
// Warning: (409-431): Assertion violation happens here
// Warning: (571-597): Assertion violation happens here
