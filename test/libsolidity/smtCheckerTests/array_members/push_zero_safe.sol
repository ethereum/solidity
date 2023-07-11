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
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
