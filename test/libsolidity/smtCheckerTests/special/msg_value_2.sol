pragma experimental SMTChecker;
contract C {
	function f() public payable {
		assert(msg.value > 0);
	}
}
// ----
// Warning 6328: (78-99): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
