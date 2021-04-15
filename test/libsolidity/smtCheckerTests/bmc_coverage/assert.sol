contract C {
	function f(uint x) public pure {
		assert(x > 0);
	}
	function g(uint x) public pure {
		require(x >= 0);
	}
	function h(uint x) public pure {
		require(x == 2);
		require(x != 2);
	}
	function i(uint x) public pure {
		if (false) {
			if (x != 2) {
			}
		}
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (49-62): BMC: Assertion violation happens here.
// Warning 6838: (111-117): BMC: Condition is always true.
// Warning 6838: (186-192): BMC: Condition is always false.
// Warning 2512: (254-260): BMC: Condition unreachable.
