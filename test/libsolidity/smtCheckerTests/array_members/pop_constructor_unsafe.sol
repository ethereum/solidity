contract C {
	uint[] a;
	constructor() {
		a.pop();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (43-50='a.pop()'): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\nTransaction trace:\nC.constructor()
