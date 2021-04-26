contract C {
	function f(bool x) public pure {
		(uint a, uint b) = x ? (10000000001, 2) : (3, 4);
		assert(a != 0);
		assert(b != 0);
		assert(a % 2 == 1);
		assert(b % 2 == 0);
	}
}
// ====
// SMTEngine: all
// ----
