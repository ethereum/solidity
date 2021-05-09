contract C {
	uint constant DEPOSIT_CONTRACT_TREE_DEPTH = 32;
	uint constant MAX_DEPOSIT_COUNT = 2**DEPOSIT_CONTRACT_TREE_DEPTH - 1;
	function f() public pure {
		assert(DEPOSIT_CONTRACT_TREE_DEPTH == 32);
		assert(MAX_DEPOSIT_COUNT == 4294967295);
		assert(MAX_DEPOSIT_COUNT == 2); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (251-281): CHC: Assertion violation happens here.\nCounterexample:\nDEPOSIT_CONTRACT_TREE_DEPTH = 32, MAX_DEPOSIT_COUNT = 4294967295\n\nTransaction trace:\nC.constructor()\nState: DEPOSIT_CONTRACT_TREE_DEPTH = 32, MAX_DEPOSIT_COUNT = 4294967295\nC.f()
