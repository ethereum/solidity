contract C {
	function f() public view {
		assert(address(this).balance == 0); // should fail because there might be funds from before deployment
		assert(address(this).balance > 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (43-77): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (148-181): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
