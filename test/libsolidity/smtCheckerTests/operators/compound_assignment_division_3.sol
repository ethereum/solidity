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
// Warning 6328: (194-213): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 2\np = 0\n\n\nTransaction trace:\nconstructor()\nf(2, 0)
