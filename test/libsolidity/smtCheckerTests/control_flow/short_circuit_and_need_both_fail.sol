pragma experimental SMTChecker;

contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g() public returns (bool) {
		x = 0;
		bool b = (f() > 0) && (f() > 0);
		assert(x == 2);
		assert(!b);
		return b;
	}
}
// ----
// Warning 6328: (225-235): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\n = false\n\nTransaction trace:\nconstructor()\nState: x = 0\ng()
