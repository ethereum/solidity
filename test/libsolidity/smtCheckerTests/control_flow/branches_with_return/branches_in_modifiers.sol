contract C {

	uint x;

	modifier check() {
		require(x == 0);
		_;
		assert(x == 1); // should fail;
		assert(x == 0); // should hold;
	}

	modifier inc() {
		if (x == 0) {
			return;
		}
		x = x + 1;
		_;
	}

	function test() check inc public {
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (70-84): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
