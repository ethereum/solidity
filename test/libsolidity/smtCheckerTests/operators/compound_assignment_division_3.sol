pragma experimental SMTChecker;
contract C {
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x == 2);
		map[p] = 10;
		map[p] /= map[p] / x;
		assert(map[p] == x);
		assert(map[p] == 0);
	}
}
// ----
// Warning 1218: (171-190): Error trying to invoke SMT solver.
// Warning 1218: (194-213): Error trying to invoke SMT solver.
// Warning 4661: (194-213): Assertion violation happens here
