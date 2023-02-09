contract C {
	struct S { uint x; }

	function g() internal pure returns (uint, S memory) {
		return (2, S(3));
	}
	function f(uint a) public pure {
		uint x;
		S memory y;
		if (a > 100) {
			(x, y) = g();
			assert(y.x == 3);
		 }
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
