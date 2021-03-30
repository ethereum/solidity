pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) singleMap;
	mapping (uint => uint)[] severalMaps;
	mapping (uint => uint8)[] severalMaps8;
	mapping (uint => uint)[][] severalMaps3d;
	constructor() {
		severalMaps.push();
		severalMaps8.push();
		severalMaps3d.push().push();
	}
	function f(mapping (uint => uint) storage map) internal {
		map[0] = 42;
		// Index accesses are safe but the assignment above makes
		// them fail because of aliasing.
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
		require(x < severalMaps.length);
		f(severalMaps[x]);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6368: (472-486): CHC: Out of bounds access happens here.
// Warning 6368: (525-541): CHC: Out of bounds access happens here.
// Warning 6368: (525-544): CHC: Out of bounds access happens here.
// Warning 6368: (655-669): CHC: Out of bounds access happens here.
// Warning 6368: (883-899): CHC: Out of bounds access happens here.
// Warning 6368: (883-902): CHC: Out of bounds access happens here.
// Warning 6328: (969-989): CHC: Assertion violation happens here.
// Warning 6368: (1062-1076): CHC: Out of bounds access might happen here.
