contract MockContract {
	bytes4 public constant SENTINEL_ANY_MOCKS = hex"01";

	constructor() {
		assert(SENTINEL_ANY_MOCKS >= 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
