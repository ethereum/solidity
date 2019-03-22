pragma experimental SMTChecker;

contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g() public {
		x = 0;
		bool b = (f() > 0) || (f() > 0);
		// This assertion should NOT fail.
		// It currently does because the SMTChecker does not
		// handle short-circuiting properly and inlines f() twice.
		assert(x == 1);
	}
}
// ----
// Warning: (101-106): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (344-358): Assertion violation happens here
