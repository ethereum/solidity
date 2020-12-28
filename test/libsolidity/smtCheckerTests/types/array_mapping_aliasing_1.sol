pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[] severalMaps;
	mapping (uint => uint8)[] severalMaps8;
	mapping (uint => uint)[][] severalMaps3d;
	function f(mapping (uint => uint) storage map) internal {
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
		f(severalMaps[x]);
	}
}
// ----
// Warning 6328: (421-452): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 38\n\n\nTransaction trace:\nconstructor()\ng(38)
