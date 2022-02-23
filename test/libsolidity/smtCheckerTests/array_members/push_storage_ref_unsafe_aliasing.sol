contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[0].push();
		a[0][0] = 16;
		uint[] storage b = a[0];
		b[0] = 32;
		// Access is safe but fails due to aliasing.
		assert(a[0][0] == 16);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// SMTIgnoreCex: yes
// ----
// Warning 6368: (188-192): CHC: Out of bounds access happens here.
// Warning 6368: (188-195): CHC: Out of bounds access happens here.
// Warning 6328: (181-202): CHC: Assertion violation happens here.
