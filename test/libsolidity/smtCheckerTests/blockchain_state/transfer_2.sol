contract C {
	address payable recipient;
	uint amount;

	function shouldHold() public {
		uint tempAmount = address(this).balance;
		recipient.transfer(tempAmount);
		recipient.transfer(amount);
	}
}
// ====
// SMTEngine: chc
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
