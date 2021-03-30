pragma experimental SMTChecker;
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
// ----
// Warning 6368: (555-560): CHC: Out of bounds access happens here.
// Warning 6368: (555-563): CHC: Out of bounds access happens here.
// Warning 6368: (589-595): CHC: Out of bounds access happens here.
// Warning 6368: (589-598): CHC: Out of bounds access happens here.
// Warning 6368: (589-601): CHC: Out of bounds access happens here.
// Warning 6368: (610-614): CHC: Out of bounds access happens here.
// Warning 6368: (731-735): CHC: Out of bounds access happens here.
// Warning 6368: (744-749): CHC: Out of bounds access happens here.
// Warning 6368: (744-752): CHC: Out of bounds access happens here.
// Warning 6368: (762-768): CHC: Out of bounds access happens here.
// Warning 6368: (762-771): CHC: Out of bounds access happens here.
// Warning 6368: (762-774): CHC: Out of bounds access happens here.
// Warning 6328: (724-781): CHC: Assertion violation happens here.
// Warning 6368: (882-886): CHC: Out of bounds access happens here.
