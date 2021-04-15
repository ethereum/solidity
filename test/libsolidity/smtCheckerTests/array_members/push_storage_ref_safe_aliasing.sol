contract C {
	uint[][] a;
	function f() public {
		a.push();
		uint[] storage b = a[0];
		b.push(8);
		assert(b[b.length - 1] == 8);
		// Safe but fails due to aliasing.
		assert(a[0][a[0].length - 1] == 8);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6368: (179-183): CHC: Out of bounds access happens here.
// Warning 6368: (184-188): CHC: Out of bounds access happens here.
// Warning 3944: (184-199): CHC: Underflow (resulting value less than 0) happens here.
// Warning 6328: (172-206): CHC: Assertion violation happens here.
