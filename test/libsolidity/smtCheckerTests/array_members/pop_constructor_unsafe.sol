contract C {
	uint[] a;
	constructor() {
		a.pop();
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 2529: (43-50): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\nTransaction trace:\nC.constructor()
