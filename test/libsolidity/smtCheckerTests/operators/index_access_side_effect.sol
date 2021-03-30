pragma experimental SMTChecker;

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
// SMTIgnoreCex: yes
// ----
// Warning 6368: (240-244): CHC: Out of bounds access happens here.
// Warning 6368: (254-258): CHC: Out of bounds access happens here.
// Warning 6368: (304-310): CHC: Out of bounds access happens here.
// Warning 6368: (325-331): CHC: Out of bounds access happens here.
// Warning 6328: (318-337): CHC: Assertion violation happens here.
