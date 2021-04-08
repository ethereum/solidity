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
		h()[2] = 4;
		assert(h()[2] == 3);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (207-211): CHC: Out of bounds access happens here.
// Warning 6368: (221-225): CHC: Out of bounds access happens here.
// Warning 6368: (271-277): CHC: Out of bounds access happens here.
// Warning 6368: (292-298): CHC: Out of bounds access happens here.
// Warning 6328: (285-304): CHC: Assertion violation happens here.
