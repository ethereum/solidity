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
