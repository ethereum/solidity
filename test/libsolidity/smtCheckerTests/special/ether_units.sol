contract D {
	function f() public pure {
		assert(1000000000000000000 wei == 1 ether);
		assert(100000000000000000 wei == 1 ether);
		assert(1000000000 wei == 1 gwei);
		assert(100000000 wei == 1 gwei);
		assert(1000000000 gwei == 1 ether);
		assert(100000000 gwei == 1 ether);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (89-130): CHC: Assertion violation happens here.
// Warning 6328: (170-201): CHC: Assertion violation happens here.
// Warning 6328: (243-276): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
