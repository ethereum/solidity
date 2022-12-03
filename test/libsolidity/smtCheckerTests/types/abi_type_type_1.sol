contract C {
	function f(bytes memory d) public pure {
		(bool a, uint x) = abi.decode(d, (bool, uint));
		assert(a == (x == 2)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (107-128): CHC: Assertion violation happens here.\nCounterexample:\n\na = false\nx = 2\n\nTransaction trace:\nC.constructor()\nC.f(d)
