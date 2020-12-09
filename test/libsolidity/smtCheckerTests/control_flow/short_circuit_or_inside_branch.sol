pragma experimental SMTChecker;

contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g(bool a) public returns (bool) {
		bool b;
		if (a) {
			x = 0;
			b = (f() > 0) || (f() > 0);
			assert(x == 1);
			assert(b);
		} else {
			x = 100;
			b = (f() == 0) || (f() > 0);
			assert(x == 102);
			// Should fail.
			assert(!b);
		}
		return b;
	}
}
// ----
// Warning 6328: (360-370): CHC: Assertion violation happens here.\nCounterexample:\nx = 102\na = false\n = false\n\nTransaction trace:\nconstructor()\nState: x = 0\ng(false)
