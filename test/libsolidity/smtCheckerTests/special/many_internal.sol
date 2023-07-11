contract C
{
	function f() public payable {
		g();
	}
	function g() internal {
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
// Warning 8417: (128-144): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 6328: (81-117): CHC: Assertion violation happens here.
// Warning 6328: (121-163): CHC: Assertion violation happens here.
// Warning 6328: (167-209): CHC: Assertion violation happens here.
// Warning 6328: (213-252): CHC: Assertion violation happens here.
// Warning 6328: (256-288): CHC: Assertion violation happens here.
// Warning 6328: (292-323): CHC: Assertion violation happens here.
// Warning 6328: (376-400): CHC: Assertion violation happens here.
// Warning 6328: (404-432): CHC: Assertion violation happens here.
// Warning 6328: (436-459): CHC: Assertion violation happens here.
