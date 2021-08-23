contract C {
	uint[] a;
	constructor() {
		a.push();
		a.push();
		a.push();
		a.push();
	}
	// Accesses are safe but oob is reported due to aliasing.
	function h() internal returns (uint[] storage) {
		if (a[2] == 0)
			a[2] = 3;
		return a;
	}
	function g() public {
		// Disabled because of Spacer nondeterminism.
		//h()[2] = 4;
		assert(h()[2] == 3);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (335-354): CHC: Assertion violation might happen here.
// Warning 4661: (335-354): BMC: Assertion violation happens here.
