contract C {
	address public x;
	address payable public y;

	function f() public view {
		address a = this.x();
		address b = this.y();
		assert(a == x); // should hold
		assert(a == address(this)); // should fail
		assert(b == y); // should hold
		assert(y == address(this)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (171-197): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\na = 0\nb = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, y = 0\nC.f()
// Warning 6328: (249-275): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\na = 0\nb = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, y = 0\nC.f()
