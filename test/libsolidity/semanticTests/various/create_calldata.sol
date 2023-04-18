contract C {
	bytes public s;
	constructor(uint256 x) {
		// Due to a bug in EVMHost, msg.data used to contain initcode and constructor arguments.
		s = msg.data;
		assert(msg.data.length == 0);
	}
}
// ----
// constructor(): 42 ->
// gas irOptimized: 147541
// gas legacy: 177488
// gas legacyOptimized: 140089
// s() -> 0x20, 0
