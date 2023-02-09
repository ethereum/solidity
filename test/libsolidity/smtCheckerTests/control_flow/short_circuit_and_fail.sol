contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g() public returns (bool) {
		x = 0;
		bool b = (f() == 0) && (f() == 0);
		assert(x == 1);
		assert(b);
		return b;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (194-203): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
