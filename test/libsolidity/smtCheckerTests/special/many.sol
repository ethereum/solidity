contract C
{
	function f() public payable {
		assert(msg.sender == block.coinbase);
		assert(block.difficulty == block.gaslimit);
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
// Warning 6328: (46-82): CHC: Assertion violation happens here.
// Warning 6328: (86-128): CHC: Assertion violation happens here.
// Warning 6328: (132-171): CHC: Assertion violation happens here.
// Warning 6328: (175-207='assert(tx.gasprice == msg.value)'): CHC: Assertion violation happens here.
// Warning 6328: (211-242='assert(tx.origin == msg.sender)'): CHC: Assertion violation happens here.
// Warning 6328: (295-319='assert(x > block.number)'): CHC: Assertion violation happens here.
// Warning 6328: (323-351='assert(block.timestamp > 10)'): CHC: Assertion violation happens here.
// Warning 6328: (355-378='assert(gasleft() > 100)'): CHC: Assertion violation happens here.
