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
// Warning 6328: (79-115): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (119-161): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (165-204): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (208-240): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (244-275): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 4984: (311-316): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (304-332): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (336-364): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (368-391): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
