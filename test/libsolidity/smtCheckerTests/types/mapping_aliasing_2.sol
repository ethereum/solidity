pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) a;
	mapping (uint => mapping (uint => uint)) maps;
	mapping (uint => mapping (uint => uint8)) maps8;
	function f(mapping (uint => uint) storage map1, mapping (uint => uint) storage map2) internal {
		require(map1[0] == 2);
		require(a[0] == 42);
		require(maps[0][0] == 42);
		require(maps8[0][0] == 42);
		map2[0] = 1;
		// Fails because map2 == map1 is possible.
		assert(map1[0] == 2);
		// Fails because map2 == a is possible.
		assert(a[0] == 42);
		// Fails because map2 == maps[0] is possible.
		assert(maps[0][0] == 42);
		// Should not fail since knowledge is erased only for mapping (uint => uint).
		assert(maps8[0][0] == 42);
		assert(map2[0] == 1);
	}
}
// ----
// Warning: (437-457): Assertion violation happens here
// Warning: (503-521): Assertion violation happens here
// Warning: (573-597): Assertion violation happens here
