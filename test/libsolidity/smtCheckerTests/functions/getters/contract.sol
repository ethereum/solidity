pragma experimental SMTChecker;

contract D {}

contract C {
	D public d;

	function f() public view {
		D e = this.d();
		assert(e == d); // should hold
		assert(address(e) == address(this)); // should fail
	}
}
// ----
// Warning 6328: (156-191): CHC: Assertion violation happens here.\nCounterexample:\nd = 0\n\n\n\nTransaction trace:\nconstructor()\nState: d = 0\nf()
