contract C {
	mapping (uint => uint) public map;

	function f() public view {
		uint y = this.map(2);
		assert(y == map[2]); // should hold
		assert(y == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (142-156): CHC: Assertion violation happens here.\nCounterexample:\n\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f()
