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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (132-188): CHC: Assertion violation happens here.
// Warning 6328: (269-324): CHC: Assertion violation happens here.
// Info 1180: Contract invariant(s) for :C:\n((prevBalance + ((- 1) * (:var 1).balances[address(this)])) <= 0)\n
