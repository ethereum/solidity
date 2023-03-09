contract C
{
	function f(address payable a) public {
		require(1000 == address(this).balance);
		require(100 == a.balance);
		a.transfer(600);
		// a == this is not possible because address(this).balance == 1000
		// and a.balance == 100,
		// so this should hold in CHC, ignoring the transfer revert.
		assert(a.balance == 700);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
