pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[2] severalMaps;
	mapping (uint => uint8)[2] severalMaps8;
	mapping (uint => uint)[2][2] severalMaps3d;
	function f(mapping (uint => uint) storage map) internal {
		// Accesses are safe but oob is reported because of aliasing.
		severalMaps[0][0] = 42;
		severalMaps8[0][0] = 42;
		severalMaps3d[0][0][0] = 42;
		map[0] = 2;
		// Should fail since map == severalMaps[0] is possible.
		assert(severalMaps[0][0] == 42);
		// Should not fail since knowledge is erased only for mapping (uint => uint).
		assert(severalMaps8[0][0] == 42);
		// Should fail since map == severalMaps3d[0][0] is possible.
		// Removed because current Spacer seg faults in cex generation.
		//assert(severalMaps3d[0][0][0] == 42);
	}
	function g(uint x) public {
		require(x < severalMaps.length);
		f(severalMaps[x]);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6368: (386-402): CHC: Out of bounds access happens here.
// Warning 6368: (386-405): CHC: Out of bounds access happens here.
// Warning 6368: (496-510): CHC: Out of bounds access happens here.
// Warning 6328: (489-520): CHC: Assertion violation happens here.
