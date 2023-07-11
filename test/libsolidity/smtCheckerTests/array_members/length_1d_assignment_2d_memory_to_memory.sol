pragma abicoder               v2;

contract C {
	function f(uint[][] memory arr) public pure {
		require(arr.length > 0);
		uint[][] memory arr2 = arr;
		assert(arr2[0].length == arr[0].length);
		assert(arr.length == arr2.length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
