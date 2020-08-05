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
		assert(x + 2 > block.number);
		assert(block.timestamp > 10);
		assert(gasleft() > 100);
	}
}
// ----
// Warning 6328: (79-115): Assertion violation happens here
// Warning 6328: (119-161): Assertion violation happens here
// Warning 6328: (165-204): Assertion violation happens here
// Warning 6328: (208-240): Assertion violation happens here
// Warning 6328: (244-275): Assertion violation happens here
// Warning 6328: (304-332): Assertion violation happens here
// Warning 6328: (336-364): Assertion violation happens here
// Warning 6328: (368-391): Assertion violation happens here
// Warning 2661: (311-316): Overflow (resulting value larger than 2**256 - 1) happens here
