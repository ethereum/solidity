pragma abicoder               v2;

contract C {
	uint[][] arr;
	function f(uint[][] memory arr2) public {
		require(arr2.length > 0);
		arr = arr2;
		assert(arr2[0].length == arr[0].length);
		assert(arr2.length == arr.length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
