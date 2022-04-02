pragma abicoder               v2;

contract C
{
	function f(
		uint[] memory a,
		uint[] memory b,
		uint[][] memory cc,
		uint8[][] memory dd,
		uint[][][] memory eee
	) public pure {
		require(a.length > 0);
		require(b.length > 0);
		require(cc.length > 0);
		require(cc[0].length > 0);
		require(dd.length > 0);
		require(dd[0].length > 0);
		require(eee.length > 0);
		require(eee[0].length > 0);
		require(eee[0][0].length > 0);
		a[0] = 2;
		// The accesses below are safe but oob is reported because of aliasing.
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
// ====
// SMTEngine: all
// ----
// Warning 6368: (523-528='cc[0]'): CHC: Out of bounds access happens here.
// Warning 6368: (523-531='cc[0][0]'): CHC: Out of bounds access happens here.
// Warning 6368: (557-563='eee[0]'): CHC: Out of bounds access happens here.
// Warning 6368: (557-566='eee[0][0]'): CHC: Out of bounds access happens here.
// Warning 6368: (557-569='eee[0][0][0]'): CHC: Out of bounds access happens here.
// Warning 6368: (578-582='b[0]'): CHC: Out of bounds access happens here.
// Warning 6368: (699-703='a[0]'): CHC: Out of bounds access happens here.
// Warning 6368: (712-717='cc[0]'): CHC: Out of bounds access happens here.
// Warning 6368: (712-720='cc[0][0]'): CHC: Out of bounds access happens here.
// Warning 6368: (730-736='eee[0]'): CHC: Out of bounds access happens here.
// Warning 6368: (730-739='eee[0][0]'): CHC: Out of bounds access happens here.
// Warning 6368: (730-742='eee[0][0][0]'): CHC: Out of bounds access happens here.
// Warning 6328: (692-749): CHC: Assertion violation happens here.
// Warning 6368: (850-854='b[0]'): CHC: Out of bounds access happens here.
