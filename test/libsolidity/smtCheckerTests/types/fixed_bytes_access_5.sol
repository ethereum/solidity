contract C {
	function f() public pure {
		bytes4 x = 0x01020304;
		bytes1 b = 0x02;
		assert(x[0] == b); // fails
		assert(x[1] == b);
		assert(x[2] == b); // fails
		assert(x[3] == b); // fails
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (87-104): CHC: Assertion violation happens here.
// Warning 6328: (138-155): CHC: Assertion violation happens here.
// Warning 6328: (168-185): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
