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
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
