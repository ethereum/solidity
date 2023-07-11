contract C {
	uint x;

	constructor() {
		x = block.timestamp + 0; // No overflow should be reported
		x = block.timestamp + 1; // Overflow should be reported here
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (107-126): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
