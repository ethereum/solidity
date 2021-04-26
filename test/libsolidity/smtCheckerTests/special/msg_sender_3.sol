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
