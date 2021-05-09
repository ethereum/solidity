contract C {
	uint z = 0;
	uint x = 2 / z;
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (36-41): CHC: Division by zero happens here.\nCounterexample:\nz = 0, x = 0\n\nTransaction trace:\nC.constructor()
