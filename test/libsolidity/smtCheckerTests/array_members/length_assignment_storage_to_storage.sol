contract C {
	uint[] arr;
	uint[] arr2;
	function f() public {
		arr2 = arr;
		assert(arr2.length == arr.length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
