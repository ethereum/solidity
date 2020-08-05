pragma experimental SMTChecker;

contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[0].push();
		a[0][0] = 16;
		uint[] storage b = a[0];
		b[0] = 32;
		assert(a[0][0] == 16);
	}
}
// ----
// Warning 6328: (167-188): Assertion violation happens here
