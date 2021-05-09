contract C {
	bytes1 public x;
	bytes3 public y;

	function f() public view {
		bytes1 a = this.x();
		bytes3 b = this.y();
		assert(a == x); // should hold
		assert(a == 'a'); // should fail
		assert(b == y); // should hold
		assert(y == "abc"); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (159-175): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\na = 0\nb = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, y = 0\nC.f()
// Warning 6328: (227-245): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\na = 0\nb = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, y = 0\nC.f()
