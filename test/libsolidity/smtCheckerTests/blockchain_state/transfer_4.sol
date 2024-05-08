contract C {
	address payable recipient;

	function f() public payable {
		require(msg.value > 1);
		recipient.transfer(1);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
