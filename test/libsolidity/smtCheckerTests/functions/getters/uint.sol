pragma experimental SMTChecker;

contract C {
	uint public x;

	function f() public view {
		uint y = this.x();
		assert(y == x); // should hold
		assert(y == 1); // should fail
	}
}
// ----
// Warning 6328: (147-161): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf()
