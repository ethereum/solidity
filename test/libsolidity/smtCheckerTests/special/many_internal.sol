contract C
{
	function f() public payable {
		g();
	}
	function g() internal {
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
// Warning 6328: (81-117): CHC: Assertion violation happens here.
// Warning 6328: (121-163): CHC: Assertion violation happens here.
// Warning 6328: (167-206): CHC: Assertion violation happens here.
// Warning 6328: (210-242='assert(tx.gasprice == msg.value)'): CHC: Assertion violation happens here.
// Warning 6328: (246-277='assert(tx.origin == msg.sender)'): CHC: Assertion violation happens here.
// Warning 6328: (330-354='assert(x > block.number)'): CHC: Assertion violation happens here.
// Warning 6328: (358-386='assert(block.timestamp > 10)'): CHC: Assertion violation happens here.
// Warning 6328: (390-413='assert(gasleft() > 100)'): CHC: Assertion violation happens here.
