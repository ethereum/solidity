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
		s.x = 1;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (123-139): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
