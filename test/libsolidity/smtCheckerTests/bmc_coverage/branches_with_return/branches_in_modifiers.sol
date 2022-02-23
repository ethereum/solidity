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
// SMTEngine: bmc
// ----
// Warning 4661: (70-84): BMC: Assertion violation happens here.
