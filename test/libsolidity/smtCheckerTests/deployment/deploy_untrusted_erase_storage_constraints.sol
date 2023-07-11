contract D {
	uint x;
}

contract C {
	uint y;
	function g() public {
		D d = new D();
		assert(y == 0); // should fail in ext calls untrusted mode
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (72-75): Unused local variable.
// Warning 8729: (78-85): Contract deployment is only supported in the trusted mode for external calls with the CHC engine.
// Warning 6328: (89-103): CHC: Assertion violation happens here.\nCounterexample:\ny = 1\nd = 0\n\nTransaction trace:\nC.constructor()\nState: y = 0\nC.g()
