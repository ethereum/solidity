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
