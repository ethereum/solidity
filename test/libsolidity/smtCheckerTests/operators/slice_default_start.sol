contract C {
	function f(bytes calldata b) external pure {
		require(b.length > 20);
		require(b[0] == 0xff);
		assert(bytes(b[:20]).length == 20);
		assert(bytes(b[:20])[0] == 0xff);
		assert(bytes(b[:20])[5] == 0xff);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (186-218): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
