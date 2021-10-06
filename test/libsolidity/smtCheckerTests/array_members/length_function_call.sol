contract C {
	uint[] arr;
	function f() public view {
		assert(arr.length == g().length);
	}
	function g() internal pure returns (uint[] memory) {
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n(arr.length <= 0)\n
