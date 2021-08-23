contract c {
	mapping(uint => uint) x;
	mapping(uint => uint) y;
	function f(bool cond) public {
		mapping(uint => uint) storage a = cond ? x : y;
		x[2] = 1;
		y[2] = 2;
		a[2] = 3;
		// False positive since aliasing is not yet supported.
		if (cond)
			assert(a[2] == x[2] && a[2] != y[2]);
		else
			assert(a[2] == y[2] && a[2] != x[2]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (255-291): CHC: Assertion violation happens here.
// Warning 6328: (303-339): CHC: Assertion violation happens here.
