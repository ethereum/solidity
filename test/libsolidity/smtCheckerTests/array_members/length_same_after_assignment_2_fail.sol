pragma experimental SMTChecker;

contract C {
	uint[][] arr;
	uint[][] arr2;
	function f() public {
		uint x = arr[2].length;
		uint y = arr[3].length;
		uint z = arr.length;
		arr[2][333] = 444;
		assert(arr[2].length != x);
		assert(arr[3].length != y);
		assert(arr.length != z);
	}
}
// ----
// Warning 6328: (198-224): Assertion violation happens here
// Warning 6328: (228-254): Assertion violation happens here
// Warning 6328: (258-281): Assertion violation happens here
