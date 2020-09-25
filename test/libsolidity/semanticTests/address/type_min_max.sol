contract test {

	function address_min_max() public pure returns (bool) {
		address a = type(address).min;
		require(a == address(0));

		address b = type(address).max;
		require(b == address(2**160 - 1));

		return true;
	}
}
// ====
// compileViaYul: also
// ----
// address_min_max() -> true
