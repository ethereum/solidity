contract C {
	constructor() payable {
		require(msg.value > 100);
	}
	function f(address payable _a, uint _v) public {
		require(_v < 10);
		_a.transfer(_v);
	}
	function inv() public view {
		//assert(address(this).balance > 0); // should fail, but commented out because of Spacer nondeterminism
		assert(address(this).balance > 80); // should fail
		assert(address(this).balance > 90); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (299-333): CHC: Assertion violation happens here.
// Warning 6328: (352-386): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor(){ msg.value: 101 }\nC.f(0x11, 5)\nC.f(0x11, 9)\nC.inv()
// Warning 1236: (141-156): BMC: Insufficient funds happens here.
