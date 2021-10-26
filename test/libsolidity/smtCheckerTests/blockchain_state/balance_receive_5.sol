contract C {
	uint sum = msg.value;
	function f() public payable {
		sum += msg.value;
	}
	function inv() public view {
		assert(address(this).balance == sum); // should fail
		assert(address(this).balance >= sum); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (122-158): CHC: Assertion violation happens here.\nCounterexample:\nsum = 0\n\nTransaction trace:\nC.constructor()\nState: sum = 0\nC.inv()
// Info 1180: Contract invariant(s) for :C:\n((sum + ((- 1) * (:var 1).balances[address(this)])) <= 0)\n
