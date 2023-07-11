contract C {
	function add(uint x, uint y) internal pure returns (uint) {
		if (y == 0)
			return x;
		if (y == 1)
			return ++x;
		if (y == 2)
			return x + 2;
		return x + y;
	}

	function f() public pure {
		assert(add(100, 0) != 100);
		assert(add(100, 1) != 101);
		assert(add(100, 2) != 102);
		assert(add(100, 100) != 200);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (211-237): CHC: Assertion violation happens here.
// Warning 6328: (241-267): CHC: Assertion violation happens here.
// Warning 6328: (271-297): CHC: Assertion violation happens here.
// Warning 6328: (301-329): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
