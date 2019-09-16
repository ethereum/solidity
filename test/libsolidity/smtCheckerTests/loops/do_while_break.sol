pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint x;
		do {
			break;
			x = 1;
		} while (x == 0);
		assert(x == 0);
	}
}
// ----
// Warning: (104-109): Unreachable code.
// Warning: (122-128): Unreachable code.
