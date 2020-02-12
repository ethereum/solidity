pragma experimental SMTChecker;

library l1 {

	uint private constant TON = 1000;
	function f1() public pure {
		assert(TON == 1000);
	}
}
