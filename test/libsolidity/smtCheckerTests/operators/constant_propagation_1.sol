pragma experimental SMTChecker;

contract C {
	uint constant DEPOSIT_CONTRACT_TREE_DEPTH = 32;
	uint constant MAX_DEPOSIT_COUNT = 2**DEPOSIT_CONTRACT_TREE_DEPTH - 1;
	function f() public pure {
		assert(DEPOSIT_CONTRACT_TREE_DEPTH == 32);
		assert(MAX_DEPOSIT_COUNT == 4294967295);
		assert(MAX_DEPOSIT_COUNT == 2); // should fail
	}
}
// ----
// Warning 6328: (284-314): CHC: Assertion violation happens here.\nCounterexample:\nDEPOSIT_CONTRACT_TREE_DEPTH = 32, MAX_DEPOSIT_COUNT = 4294967295\n\n\n\nTransaction trace:\nconstructor()\nState: DEPOSIT_CONTRACT_TREE_DEPTH = 32, MAX_DEPOSIT_COUNT = 4294967295\nf()
