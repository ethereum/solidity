pragma experimental SMTChecker;
contract C {
	function f(int x, int y) public pure {
		require(y == -10);
		require(x == 100);
		int z1 = x % y;
		int z2 = x % -y;
		assert(z1 == z2);
	}
}
// ----
