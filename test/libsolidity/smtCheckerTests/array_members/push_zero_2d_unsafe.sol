contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[a.length - 1].push();
		assert(a[a.length - 1][0] == 100);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (89-122): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
