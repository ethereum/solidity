pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[2] severalMaps;
	mapping (uint => uint8)[2] severalMaps8;
	mapping (uint => uint)[2][2] severalMaps3d;
	function f(mapping (uint => uint) storage map) internal {
		map[0] = 42;
		require(severalMaps[0][0] == 42);
		require(severalMaps8[0][0] == 42);
		require(severalMaps3d[0][0][0] == 42);
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
}
// ----
// Warning: (811-831): Assertion violation happens here
