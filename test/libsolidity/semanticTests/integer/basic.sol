contract C {
	function basic() public pure returns(bool) {
		uint uint_min = type(uint).min;
		require(uint_min == 0);

		uint uint_max = type(uint).max;
		require(uint_max == 2**256 - 1);
		require(uint_max == 115792089237316195423570985008687907853269984665640564039457584007913129639935);

		int int_min = type(int).min;
		require(int_min == -2**255);
		require(int_min == -57896044618658097711785492504343953926634992332820282019728792003956564819968);

		int int_max = type(int).max;
		require(int_max == 2**255 -1);
		require(int_max == 57896044618658097711785492504343953926634992332820282019728792003956564819967);

		return true;
	}
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// basic() -> true
