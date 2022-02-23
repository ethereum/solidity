contract C {
	uint[] a;
	function f(uint x) public {
		a.push(x);
		assert(a[a.length - 1] == x);
	}
}
// ====
// SMTEngine: all
// ----
