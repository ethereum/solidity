contract C {
	uint[] arr;
	function f() public view {
		uint[] memory marr = arr;
		assert(marr.length == arr.length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
