contract C {
	function f(int a, uint b) public view {
		a >>= tx.gasprice;
		require(a == 16 && b == 2);
		a >>= b;
		assert(a == 4); // should hold
	}
}
// ====
// SMTEngine: all
// ----
