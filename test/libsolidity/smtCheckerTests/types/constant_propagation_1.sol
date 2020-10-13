pragma experimental SMTChecker;

contract C {
	uint constant DEPOSIT_CONTRACT_TREE_DEPTH = 32;
	uint constant MAX_DEPOSIT_COUNT = 2**DEPOSIT_CONTRACT_TREE_DEPTH - 1;
	function f() public pure {
		assert(DEPOSIT_CONTRACT_TREE_DEPTH == 32);
		assert(MAX_DEPOSIT_COUNT == 4294967295);
	}
}
