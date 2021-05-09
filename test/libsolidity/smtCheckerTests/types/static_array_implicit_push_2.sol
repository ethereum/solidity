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
