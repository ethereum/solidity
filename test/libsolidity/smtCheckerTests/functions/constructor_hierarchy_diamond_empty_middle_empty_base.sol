contract C {
	uint a;
	constructor() {
		a = 2;
	}
}

contract B is C {
}

contract B2 is C {
	constructor() {
		assert(a == 2);
	}
}

contract A is B, B2 {
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
