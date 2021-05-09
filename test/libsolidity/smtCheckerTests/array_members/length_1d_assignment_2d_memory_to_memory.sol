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
