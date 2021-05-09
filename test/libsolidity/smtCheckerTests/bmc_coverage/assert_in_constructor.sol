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
