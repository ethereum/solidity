contract MockContract {
	bytes4 public constant SENTINEL_ANY_MOCKS = hex"01";
	mapping(bytes4 => bytes4) methodIdMocks;

	constructor() {
		methodIdMocks[SENTINEL_ANY_MOCKS] = 0;
	}
}
// ====
// SMTEngine: all
// ----
