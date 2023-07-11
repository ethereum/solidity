contract C {
	uint[][] a;
	function f(uint[1][] memory x) public {
		require(x.length > 2);
		a.push(x[2]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
