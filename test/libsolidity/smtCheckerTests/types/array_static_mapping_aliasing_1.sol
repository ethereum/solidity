contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[2] severalMaps;
	mapping (uint => uint8)[2] severalMaps8;
	mapping (uint => uint)[2][2] severalMaps3d;
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
		assert(severalMaps3d[0][0][0] == 42);
	}
	function g(uint x) public {
		require(x < severalMaps.length);
		f(severalMaps[x]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTTargets: assert
// ----
// Warning 6328: (392-423): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)\n    C.f(map) -- counterexample incomplete; parameter name used instead of value -- internal call
// Warning 6328: (606-642): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)\n    C.f(map) -- counterexample incomplete; parameter name used instead of value -- internal call
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
