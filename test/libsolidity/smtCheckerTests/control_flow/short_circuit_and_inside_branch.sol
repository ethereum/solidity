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
			b = (f() == 0) && (f() == 0);
			assert(x == 1);
			assert(!b);
		} else {
			x = 100;
			b = (f() > 0) && (f() > 0);
			assert(x == 102);
			// Should fail.
			assert(!b);
		}
		return b;
	}
}
// ----
// Warning: (101-106): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (362-372): Assertion violation happens here
