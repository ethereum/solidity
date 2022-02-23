contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[a.length - 1].push();
		assert(a[a.length - 1][0] == 0);
	}
}
// ====
// SMTEngine: all
// ----
