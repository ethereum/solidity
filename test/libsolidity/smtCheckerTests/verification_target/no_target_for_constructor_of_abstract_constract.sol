abstract contract A {
	constructor() {
		assert(false); // A cannot be deployed, so this should not be reported
	}
}
// ====
// SMTEngine: all
// ----
