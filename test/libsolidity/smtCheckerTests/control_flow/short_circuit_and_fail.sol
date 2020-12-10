pragma experimental SMTChecker;

contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g() public returns (bool) {
		x = 0;
		bool b = (f() == 0) && (f() == 0);
		assert(x == 1);
		assert(b);
		return b;
	}
}
// ----
// Warning 6328: (227-236): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n = false\n\nTransaction trace:\nconstructor()\nState: x = 0\ng()
