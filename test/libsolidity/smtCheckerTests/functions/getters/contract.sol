contract D {}

contract C {
	D public d;

	function f() public view {
		D e = this.d();
		assert(e == d); // should hold
		assert(address(e) == address(this)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (123-158): CHC: Assertion violation happens here.\nCounterexample:\nd = 0\ne = 0\n\nTransaction trace:\nC.constructor()\nState: d = 0\nC.f()
