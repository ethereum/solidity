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
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (162-181): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 2\np = 0\n\nTransaction trace:\nC.constructor()\nC.f(2, 0)
