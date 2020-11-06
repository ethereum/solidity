pragma experimental SMTChecker;
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
// Warning 4661: (81-94): BMC: Assertion violation happens here.
// Warning 6838: (143-149): BMC: Condition is always true.
// Warning 6838: (218-224): BMC: Condition is always false.
// Warning 2512: (286-292): BMC: Condition unreachable.
