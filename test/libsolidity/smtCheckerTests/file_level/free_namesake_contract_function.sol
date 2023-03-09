function f() pure returns (uint) { return 1337; }

contract C {
	function g() public pure {
		assert(f() == 42); // should hold
		assert(f() == 1337); // should fail
	}
	function f() internal pure returns (uint) { return 42; }
}
// ====
// SMTEngine: all
// ----
// Warning 2519: (170-226): This declaration shadows an existing declaration.
// Warning 6328: (130-149): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
