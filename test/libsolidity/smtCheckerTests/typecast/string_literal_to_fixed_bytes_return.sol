contract C {
    function g() public pure returns (bytes32 val) { return "abc"; }
    function f1() public pure returns (bytes32 val) { return g(); }

	function a() public pure {
		assert(f1() == "abc");
		assert(f1() == "cde");
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (206-227): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
