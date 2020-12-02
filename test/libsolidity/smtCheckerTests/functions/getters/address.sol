pragma experimental SMTChecker;

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
// ----
// Warning 6328: (204-230): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0, y = 0\nf()
// Warning 6328: (282-308): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, y = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0, y = 0\nf()
