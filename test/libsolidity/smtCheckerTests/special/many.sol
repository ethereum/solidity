pragma experimental SMTChecker;

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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (79-115): CHC: Assertion violation happens here.
// Warning 6328: (119-161): CHC: Assertion violation happens here.
// Warning 6328: (165-204): CHC: Assertion violation happens here.
// Warning 6328: (208-240): CHC: Assertion violation happens here.
// Warning 6328: (244-275): CHC: Assertion violation happens here.
// Warning 6328: (328-352): CHC: Assertion violation happens here.
// Warning 6328: (356-384): CHC: Assertion violation happens here.
// Warning 6328: (388-411): CHC: Assertion violation happens here.
