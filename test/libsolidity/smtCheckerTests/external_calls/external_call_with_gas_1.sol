library L {
	function f() public view {
		(bool success, ) = address(10).staticcall{gas: 3}("");
		require(success);
	}
}
// ====
// SMTEngine: all
// ----
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
