pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint x;
		do {
			continue;
			x = 1;
		} while (x == 0);
		assert(x == 0);
	}
}
// ----
// Warning: (107-112): Unreachable code.
