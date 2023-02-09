contract C {
	uint[][] a;
	function f() public {
		a.push();
		a.push();
		a[0].push();
		a[1].pop();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (90-100): CHC: Empty array "pop" happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
