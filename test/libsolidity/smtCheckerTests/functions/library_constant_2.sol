library l1 {

	uint private constant TON = 1000;
	function f1() public pure {
		assert(TON == 1000);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
