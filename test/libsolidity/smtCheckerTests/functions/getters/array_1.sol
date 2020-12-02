pragma experimental SMTChecker;

contract C {
	uint[] public a;

	function f() public view {
		uint y = this.a(2);
		assert(y == a[2]); // should hold
		assert(y == 1); // should fail
	}
}
// ----
// Warning 6328: (153-167): CHC: Assertion violation happens here.\nCounterexample:\na = []\n\n\n\nTransaction trace:\nconstructor()\nState: a = []\nf()
