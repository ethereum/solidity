contract C {
	mapping (bytes16 => uint) public m;

	function f() public view {
		uint y = this.m("foo");
		assert(y == m["foo"]); // should hold
		assert(y == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (147-161): CHC: Assertion violation happens here.\nCounterexample:\n\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f()
