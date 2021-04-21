contract MockContract {
	bytes4 public constant SENTINEL_ANY_MOCKS = hex"01";

	constructor() {
		assert(SENTINEL_ANY_MOCKS >= 0);
	}
}
// ====
// SMTEngine: all
// ----
