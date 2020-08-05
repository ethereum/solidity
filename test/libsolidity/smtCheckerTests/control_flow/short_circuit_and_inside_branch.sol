pragma experimental SMTChecker;

contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g() public returns (bool) {
		bool b;
			x = 100;
			b = f() > 0;
			assert(x == 102);
			// Should fail.
			assert(!b);
		return b;
	}
}
// ----
// Warning 6328: (202-218): Assertion violation happens here
// Warning 6328: (242-252): Assertion violation happens here
// Warning 2661: (101-106): Overflow (resulting value larger than 2**256 - 1) happens here
