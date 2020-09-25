pragma experimental SMTChecker;
contract C {
	function i() public pure returns (uint d) {
		if (0==0)
			(d) = 13;
		assert(d == 13);
	}
}
// ----
// Warning 6838: (96-100): BMC: Condition is always true.
