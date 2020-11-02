pragma experimental SMTChecker;
contract C {
	function f(uint x, uint y) public pure {
		x = 7;
		y = 2;
		assert(x / y == 3);
	}
}
// ----
