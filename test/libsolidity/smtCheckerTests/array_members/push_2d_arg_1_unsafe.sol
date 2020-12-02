pragma experimental SMTChecker;

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
// SMTIgnoreCex: yes
// ----
// Warning 3944: (162-177): CHC: Underflow (resulting value less than 0) happens here.
// Warning 6328: (150-184): CHC: Assertion violation happens here.
