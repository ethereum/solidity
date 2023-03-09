contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g(bool a) public returns (bool) {
		bool b;
		if (a) {
			x = 0;
			b = (f() > 0) || (f() > 0);
			assert(x == 1);
			assert(b);
		} else {
			x = 100;
			b = (f() == 0) || (f() > 0);
			assert(x == 102);
			// Should fail.
			assert(!b);
		}
		return b;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (327-337): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
