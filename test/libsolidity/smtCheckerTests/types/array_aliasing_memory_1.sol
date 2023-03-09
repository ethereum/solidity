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
// Warning 6368: (523-528): CHC: Out of bounds access happens here.
// Warning 6368: (523-531): CHC: Out of bounds access happens here.
// Warning 6368: (557-563): CHC: Out of bounds access happens here.
// Warning 6368: (557-566): CHC: Out of bounds access happens here.
// Warning 6368: (557-569): CHC: Out of bounds access happens here.
// Warning 6368: (578-582): CHC: Out of bounds access happens here.
// Warning 6368: (699-703): CHC: Out of bounds access happens here.
// Warning 6368: (712-717): CHC: Out of bounds access happens here.
// Warning 6368: (712-720): CHC: Out of bounds access happens here.
// Warning 6368: (730-736): CHC: Out of bounds access happens here.
// Warning 6368: (730-739): CHC: Out of bounds access happens here.
// Warning 6368: (730-742): CHC: Out of bounds access happens here.
// Warning 6328: (692-749): CHC: Assertion violation happens here.
// Warning 6368: (850-854): CHC: Out of bounds access happens here.
// Info 1391: CHC: 12 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
