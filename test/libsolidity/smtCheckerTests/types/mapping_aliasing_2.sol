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
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (364-384): CHC: Assertion violation happens here.
// Warning 6328: (430-448): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
