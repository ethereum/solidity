pragma experimental SMTChecker;
contract C {
	function f(uint x, uint y) public pure {
		x = 7;
		y = 2;
		assert(x / y == 3);
	}
}
// ----
// Warning 1218: (107-125): Error trying to invoke SMT solver.
