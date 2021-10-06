contract C {
	function f(address[2] memory a) public pure {
		assert(a.length == 2); // should hold
		assert(a.length < 2); // should fail
		assert(a.length > 2); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (102-122): CHC: Assertion violation happens here.\nCounterexample:\n\na = [0x09, 0x09]\n\nTransaction trace:\nC.constructor()\nC.f([0x09, 0x09])
// Warning 6328: (141-161): CHC: Assertion violation happens here.\nCounterexample:\n\na = [0x09, 0x09]\n\nTransaction trace:\nC.constructor()\nC.f([0x09, 0x09])
