pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint x;
		do {
			break;
			x = 1;
		} while (x == 0);
		assert(x == 1);
	}
}
// ----
// Warning: (104-109): Unreachable code.
// Warning: (122-128): Unreachable code.
// Warning: (133-147): Assertion violation happens here
