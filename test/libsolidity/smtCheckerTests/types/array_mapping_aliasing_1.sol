contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[] severalMaps;
	mapping (uint => uint8)[] severalMaps8;
	mapping (uint => uint)[][] severalMaps3d;
	function p() public {
		severalMaps.push();
		severalMaps8.push();
		severalMaps3d.push().push();
	}
	function f(mapping (uint => uint) storage map) internal {
		require(severalMaps.length > 0);
		require(severalMaps8.length > 0);
		require(severalMaps3d.length > 0);
		require(severalMaps3d[0].length > 0);
		severalMaps[0][0] = 42;
		severalMaps8[0][0] = 42;
		severalMaps3d[0][0][0] = 42;
		map[0] = 2;
		// Should fail since map == severalMaps[0] is possible.
		// Access is safe but oob is reported because of aliasing.
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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (706-720): CHC: Out of bounds access happens here.
// Warning 6328: (699-730): CHC: Assertion violation happens here.
// Info 1391: CHC: 8 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
