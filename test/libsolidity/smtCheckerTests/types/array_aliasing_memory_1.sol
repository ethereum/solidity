pragma experimental SMTChecker;
pragma experimental ABIEncoderV2;

contract C
{
	function f(
		uint[] memory a,
		uint[] memory b,
		uint[][] memory cc,
		uint8[][] memory dd,
		uint[][][] memory eee
	) public pure {
		a[0] = 2;
		cc[0][0] = 50;
		dd[0][0] = 10;
		eee[0][0][0] = 50;
		b[0] = 1;
		// Fails because
		// b == a is possible
		// b == cc[0] is possible
		// b == ee[0][0] is possible
		assert(a[0] == 2 || cc[0][0] == 50 || eee[0][0][0] == 50);
		// Should not fail since knowledge is erased only for uint[].
		assert(dd[0][0] == 10);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (400-457): Assertion violation happens here
