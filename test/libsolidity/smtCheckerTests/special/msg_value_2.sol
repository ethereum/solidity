contract C {
	function f() public payable {
		assert(msg.value > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (46-67): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
