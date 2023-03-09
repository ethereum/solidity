contract c {
	bytes10[6] data2;
	function test() public view returns (bytes10 r2) {
		r2 = data2[4][5];
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
