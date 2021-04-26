contract C {
	function f(int x, int y) public pure {
		require(y != 0);
		require(x == 42);
		int z1 = x % y;
		int z2 = -x % y;
		assert(z1 == -z2);
		assert((x >= 0 && z1 >=0) || (x <= 0 && z1 <= 0));
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (131-148): CHC: Assertion violation might happen here.
