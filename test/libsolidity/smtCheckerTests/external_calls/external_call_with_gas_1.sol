library L {
	function f() public view {
		(bool success, ) = address(10).staticcall{gas: 3}("");
		require(success);
	}
}
// ====
// SMTEngine: all
// ----
