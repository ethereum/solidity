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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
