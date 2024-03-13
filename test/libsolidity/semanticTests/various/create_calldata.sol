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
// gas irOptimized: 68697
// gas irOptimized code: 74800
// gas legacy: 78445
// gas legacy code: 95400
// gas legacyOptimized: 68661
// gas legacyOptimized code: 69000
// s() -> 0x20, 0
