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
// SMTIgnoreOS: macos
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
