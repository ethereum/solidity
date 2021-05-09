contract C {
	uint[] a;
	function f() public {
		a.push();
		assert(a[a.length - 1] == 0);
	}
}
// ====
// SMTEngine: all
// ----
