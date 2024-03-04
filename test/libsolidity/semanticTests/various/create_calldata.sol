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
// gas irOptimized: 68378
// gas irOptimized code: 77200
// gas legacy: 78445
// gas legacy code: 95400
// gas legacyOptimized: 68677
// gas legacyOptimized code: 69200
// s() -> 0x20, 0
