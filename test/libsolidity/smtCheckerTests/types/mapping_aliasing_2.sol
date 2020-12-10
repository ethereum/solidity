pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) a;
	mapping (uint => mapping (uint => uint)) maps;
	mapping (uint => mapping (uint => uint8)) maps8;
	function f(mapping (uint => uint) storage map1, mapping (uint => uint) storage map2) internal {
		map1[0] = 2;
		a[0] = 42;
		maps[0][0] = 42;
		maps8[0][0] = 42;
		map2[0] = 1;
		// Fails because map2 == map1 is possible.
		assert(map1[0] == 2);
		// Fails because map2 == a is possible.
		assert(a[0] == 42);
		// Fails because map2 == maps[0] is possible.
		// Removed because current Spacer seg faults in cex generation.
		//assert(maps[0][0] == 42);
		// Should not fail since knowledge is erased only for mapping (uint => uint).
		assert(maps8[0][0] == 42);
		assert(map2[0] == 1);
	}

	function g(bool b, uint x, uint y) public {
		if (b)
			f(a, maps[y]);
		else
			f(maps[x], maps[y]);
	}
}
// ----
// Warning 6328: (397-417): CHC: Assertion violation happens here.\nCounterexample:\n\nb = true\nx = 0\ny = 0\n\n\nTransaction trace:\nconstructor()\ng(true, 0, 0)
// Warning 6328: (463-481): CHC: Assertion violation happens here.\nCounterexample:\n\nb = true\nx = 0\ny = 0\n\n\nTransaction trace:\nconstructor()\ng(true, 0, 0)
