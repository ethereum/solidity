pragma experimental SMTChecker;
contract C {
	function f(int x, int y) public pure {
		x = 7;
		y = 2;
		assert(x / y == 3);
	}
}
// ----
