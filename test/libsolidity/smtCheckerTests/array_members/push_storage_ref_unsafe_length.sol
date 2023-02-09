contract C {
	uint[][] a;
	uint[][][] c;
	uint[] d;
	constructor() {
		c.push().push().push();
		d.push(); d.push();
	}
	function f() public {
		a.push();
		uint[] storage b = a[0];
		// Access is safe but oob reported due to aliasing.
		c[0][0][0] = 12;
		// Access is safe but oob reported due to aliasing.
		d[1] = 7;
		b.push(8);
		assert(a[0].length == 0);
		// Safe but knowledge about `c` is erased because `b` could be pointing to `c[x][y]`.
		// Access is safe but oob reported due to aliasing.
		assert(c[0][0][0] == 12);
		// Safe but knowledge about `d` is erased because `b` could be pointing to `d`.
		// Removed assertion because current Spacer seg faults in cex generation.
		//assert(d[1] == 7);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6368: (238-242): CHC: Out of bounds access happens here.
// Warning 6368: (238-245): CHC: Out of bounds access might happen here.
// Warning 6368: (238-248): CHC: Out of bounds access might happen here.
// Warning 6368: (311-315): CHC: Out of bounds access happens here.
// Warning 6368: (343-347): CHC: Out of bounds access happens here.
// Warning 6328: (336-360): CHC: Assertion violation happens here.
// Warning 6368: (513-517): CHC: Out of bounds access happens here.
// Warning 6368: (513-520): CHC: Out of bounds access happens here.
// Warning 6368: (513-523): CHC: Out of bounds access happens here.
// Warning 6328: (506-530): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
