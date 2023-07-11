contract C
{
	function f() public payable {
		assert(msg.sender == block.coinbase);
		assert(block.difficulty == block.gaslimit);
		assert(block.prevrandao == block.gaslimit);
		assert(block.number == block.timestamp);
		assert(tx.gasprice == msg.value);
		assert(tx.origin == msg.sender);
		uint x = block.number;
		unchecked { x += 2; }
		assert(x > block.number);
		assert(block.timestamp > 10);
		assert(gasleft() > 100);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 8417: (93-109): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 6328: (46-82): CHC: Assertion violation happens here.
// Warning 6328: (86-128): CHC: Assertion violation happens here.
// Warning 6328: (132-174): CHC: Assertion violation happens here.
// Warning 6328: (178-217): CHC: Assertion violation happens here.
// Warning 6328: (221-253): CHC: Assertion violation happens here.
// Warning 6328: (257-288): CHC: Assertion violation happens here.
// Warning 6328: (341-365): CHC: Assertion violation happens here.
// Warning 6328: (369-397): CHC: Assertion violation happens here.
// Warning 6328: (401-424): CHC: Assertion violation happens here.
