pragma experimental SMTChecker;

contract C {
	uint[][] a;
	uint[][][] c;
	uint[] d;
	function f() public {
		a.push();
		uint[] storage b = a[0];
		c[0][0][0] = 12;
		d[5] = 7;
		b.push(8);
		assert(a[0].length == 0);
		// Safe but knowledge about `c` is erased because `b` could be pointing to `c[x][y]`.
		assert(c[0][0][0] == 12);
		// Safe but knowledge about `d` is erased because `b` could be pointing to `d`.
		assert(d[5] == 7);
	}
}
// ----
// Warning: (193-217): Assertion violation happens here
// Warning: (309-333): Assertion violation happens here
// Warning: (419-436): Assertion violation happens here
