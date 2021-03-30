pragma experimental SMTChecker;

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
// ----
// Warning 6368: (271-275): CHC: Out of bounds access happens here.
// Warning 6368: (271-278): CHC: Out of bounds access might happen here.
// Warning 6368: (271-281): CHC: Out of bounds access might happen here.
// Warning 6368: (344-348): CHC: Out of bounds access happens here.
// Warning 6368: (376-380): CHC: Out of bounds access happens here.
// Warning 6328: (369-393): CHC: Assertion violation happens here.
// Warning 6368: (546-550): CHC: Out of bounds access happens here.
// Warning 6368: (546-553): CHC: Out of bounds access happens here.
// Warning 6368: (546-556): CHC: Out of bounds access happens here.
// Warning 6328: (539-563): CHC: Assertion violation happens here.
