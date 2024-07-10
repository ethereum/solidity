contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[2] severalMaps;
	mapping (uint => uint8)[2] severalMaps8;
	mapping (uint => uint)[2][2] severalMaps3d;
	function f(mapping (uint => uint) storage map) internal {
		map[0] = 42;
		severalMaps[0][0] = 42;
		severalMaps8[0][0] = 42;
		severalMaps3d[0][0][0] = 42;
		singleMap[0] = 2;
		// Should not fail since singleMap == severalMaps[0] is not possible.
		assert(severalMaps[0][0] == 42);
		// Should not fail since knowledge is erased only for mapping (uint => uint).
		assert(severalMaps8[0][0] == 42);
		// Should not fail since singleMap == severalMaps3d[0][0] is not possible.
		assert(severalMaps3d[0][0][0] == 42);
		// Should fail since singleMap == map is possible.
		assert(map[0] == 42);
	}
	function g(uint x) public {
		require(x < 2);
		f(severalMaps3d[x][0]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTTargets: assert
// ----
// Warning 6328: (748-768): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)\n    C.f(map) -- counterexample incomplete; parameter name used instead of value -- internal call
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
