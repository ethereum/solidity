contract C {

	struct S {
		uint x;
	}
	S s;

	function check() public {
		require(s.x == 0);
		conditional_increment();
		assert(s.x == 1); // should fail;
		assert(s.x == 0); // should hold;
	}

	function conditional_increment() internal {
		if (s.x == 0) {
			return;
		}
		s = S(1);
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (123-139): BMC: Assertion violation happens here.
