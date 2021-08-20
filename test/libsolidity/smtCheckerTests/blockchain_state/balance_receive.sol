contract C {
	uint prevBalance;
	constructor() payable {
		prevBalance = address(this).balance;
	}
	function f() public payable {
		assert(address(this).balance == prevBalance + msg.value); // should fail because there might be funds from selfdestruct/block.coinbase
		assert(address(this).balance < prevBalance + msg.value); // should fail
		assert(address(this).balance >= prevBalance + msg.value); // should hold
		prevBalance = address(this).balance;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (132-188): CHC: Assertion violation happens here.\nCounterexample:\nprevBalance = 0\n\nTransaction trace:\nC.constructor()\nState: prevBalance = 0\nC.f(){ value: 168 }
// Warning 6328: (269-324): CHC: Assertion violation happens here.\nCounterexample:\nprevBalance = 0\n\nTransaction trace:\nC.constructor()\nState: prevBalance = 0\nC.f(){ value: 1201 }
