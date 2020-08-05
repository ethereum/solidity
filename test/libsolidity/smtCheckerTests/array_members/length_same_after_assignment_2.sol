pragma experimental SMTChecker;

contract C {
	uint[][] arr;
	uint[][] arr2;
	function f() public {
		uint x = arr[2].length;
		uint y = arr[3].length;
		uint z = arr.length;
		arr[2][333] = 444;
		assert(arr[2].length == x);
		assert(arr[3].length == y);
		assert(arr.length == z);
	}
}
