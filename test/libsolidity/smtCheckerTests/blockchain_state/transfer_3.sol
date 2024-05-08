contract C {
	address payable recipient;

	function shouldFail() public {
		recipient.transfer(1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8656: (76-97): CHC: Insufficient funds happens here.
