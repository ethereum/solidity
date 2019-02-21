pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[] severalMaps;
	mapping (uint => uint8)[] severalMaps8;
	mapping (uint => uint)[][] severalMaps3d;
	function f(mapping (uint => uint) storage map) internal {
		require(severalMaps[0][0] == 42);
		require(severalMaps8[0][0] == 42);
		require(severalMaps3d[0][0][0] == 42);
		map[0] = 2;
		// Should fail since map == severalMaps[0] is possible.
		assert(severalMaps[0][0] == 42);
		// Should not fail since knowledge is erase only for mapping (uint => uint).
		assert(severalMaps8[0][0] == 42);
		// Should fail since map == severalMaps3d[0][0] is possible.
		assert(severalMaps3d[0][0][0] == 42);
	}
}
// ----
// Warning: (451-482): Assertion violation happens here
// Warning: (664-700): Assertion violation happens here
