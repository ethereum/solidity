contract C {

	function f() public view {
		assert(msg.sender >= address(0)); // should hold
		assert(msg.sender <= address(2**160-1)); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
