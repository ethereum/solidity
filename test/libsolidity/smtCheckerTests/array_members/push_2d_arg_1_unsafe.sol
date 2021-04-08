contract C {
	uint[][] a;
	function f(uint[] memory x, uint y) public {
		a.push(x);
		a[0].push(y);
		a[0].pop();
		assert(a[0][a[0].length - 1] == y);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 3944: (129-144): CHC: Underflow (resulting value less than 0) happens here.
// Warning 6328: (117-151): CHC: Assertion violation happens here.
