pragma experimental SMTChecker;

contract C {
	byte public x;
	bytes3 public y;

	function f() public view {
		byte a = this.x();
		bytes3 b = this.y();
		assert(a == x); // should hold
		assert(a == 'a'); // should fail
		assert(b == y); // should hold
		assert(y == "abc"); // should fail
	}
}
// ----
// Warning 6328: (188-204): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0, y = 0\nf()
// Warning 6328: (256-274): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0, y = 0\nf()
