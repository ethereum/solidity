contract C {

	function f(bytes1 b) public pure {
		bytes1 c = hex"7f";
		require(b > c);
		assert(uint8(b) > 127); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
