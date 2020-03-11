pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[] severalMaps;
	mapping (uint => uint8)[] severalMaps8;
	mapping (uint => uint)[][] severalMaps3d;
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
		f(severalMaps[x]);
	}
}
// ----
// Warning: (777-797): Assertion violation happens here
// Warning: (777-797): Assertion violation happens here
