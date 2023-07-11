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
// Warning 6328: (251-281): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
