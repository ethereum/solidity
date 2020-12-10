pragma experimental SMTChecker;

contract C {
	uint[][] public a;

	function f() public view {
		uint y = this.a(2,3);
		assert(y == a[2][3]); // should hold
		assert(y == 1); // should fail
	}
}
// ----
// Warning 6328: (160-174): CHC: Assertion violation happens here.\nCounterexample:\na = []\n\n\n\nTransaction trace:\nconstructor()\nState: a = []\nf()
