contract C {
	function f(uint b) public pure {
		require(b < 3);
		uint c = (b > 0) ? b++ : ++b;
		assert(c == 0);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (99-113): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
