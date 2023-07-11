contract C {
	uint x = initX();

	function initX() internal pure returns (uint) {
		return 42;
	}
}

contract D is C {
	uint y;

	constructor() {
		assert(x == 42);
		y = x;
	}
}
// ====
// SMTEngine: bmc
// ----
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
