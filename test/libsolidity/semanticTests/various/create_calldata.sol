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
// gas irOptimized: 68387
// gas irOptimized code: 69000
// gas legacy: 78348
// gas legacy code: 90200
// gas legacyOptimized: 68548
// gas legacyOptimized code: 64600
// gas ssaCFGOptimized: 68163
// gas ssaCFGOptimized code: 67800
// s() -> 0x20, 0
