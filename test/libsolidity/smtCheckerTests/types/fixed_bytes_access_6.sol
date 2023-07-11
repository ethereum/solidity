contract C {
	function f() public pure {
		bytes4 x = 0x01020304;
		bytes1 b = x[3];
		assert(b == b[0]);
		assert(b == b[0][0]);
		assert(b == b[0][0][0][0][0][0][0][0][0][0][0]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 18 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
