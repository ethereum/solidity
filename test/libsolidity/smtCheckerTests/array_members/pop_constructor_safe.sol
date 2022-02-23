contract C {
	uint[] a;
	constructor() {
		a.push();
		a.pop();
	}
}
// ====
// SMTEngine: all
// ----
