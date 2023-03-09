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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
