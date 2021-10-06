contract C {
	function f(bytes memory d) public pure {
		assert(abi.decode(d, (bool))); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (57-86): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f(d)
