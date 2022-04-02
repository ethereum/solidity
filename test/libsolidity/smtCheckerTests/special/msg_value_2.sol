contract C {
	function f() public payable {
		assert(msg.value > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (46-67='assert(msg.value > 0)'): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f(){ msg.value: 0 }
