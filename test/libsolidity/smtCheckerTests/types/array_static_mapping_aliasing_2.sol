pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[2] severalMaps;
	mapping (uint => uint8)[2] severalMaps8;
	mapping (uint => uint)[2][2] severalMaps3d;
	function f(mapping (uint => uint) storage map) internal {
		// Accesses are safe but reported as unsafe due to aliasing.
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
		// Removed because of Spacer nondeterminism.
		//assert(severalMaps3d[0][0][0] == 42);
		// Should fail since singleMap == map is possible.
		assert(map[0] == 42);
	}
	function g(uint x) public {
		require(x < 2);
		f(severalMaps3d[x][0]);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6368: (347-361): CHC: Out of bounds access happens here.
// Warning 6368: (400-416): CHC: Out of bounds access happens here.
// Warning 6368: (400-419): CHC: Out of bounds access happens here.
// Warning 6368: (530-544): CHC: Out of bounds access happens here.
// Warning 6328: (893-913): CHC: Assertion violation happens here.
// Warning 6368: (969-985): CHC: Out of bounds access might happen here.
// Warning 6368: (969-988): CHC: Out of bounds access might happen here.
