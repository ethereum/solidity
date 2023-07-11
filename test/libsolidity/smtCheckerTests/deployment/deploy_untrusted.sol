contract D {
	uint x;
	function f() public view returns (uint) { return x; }
}

contract C {
	function g() public {
		D d = new D();
		uint y = d.f();
		assert(y == 0); // should fail in ext calls untrusted mode
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8729: (124-131): Contract deployment is only supported in the trusted mode for external calls with the CHC engine.
// Warning 6328: (153-167): CHC: Assertion violation happens here.\nCounterexample:\n\nd = 0\ny = 1\n\nTransaction trace:\nC.constructor()\nC.g()\n    d.f() -- untrusted external call
