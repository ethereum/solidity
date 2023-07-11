contract C {
	address owner;

	constructor() {
		owner = msg.sender;
		assert(owner >= address(0)); // should hold
	}
}

contract D {
	address owner;

	constructor() {
		unchecked {
			owner = msg.sender;
			assert(owner >= address(0)); // should hold
		}
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
