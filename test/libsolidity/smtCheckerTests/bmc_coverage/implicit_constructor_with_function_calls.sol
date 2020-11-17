pragma experimental SMTChecker;

contract C {
	uint x = initX();
	uint y = initY();

    function initX() internal pure returns (uint) {
		return 42;
	}

	function initY() internal view returns (uint) {
		assert(x == 42);
		return x;
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (205-220): BMC: Assertion violation happens here.
