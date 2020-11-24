pragma experimental SMTChecker;
pragma abicoder               v2;

contract C {
	uint[][] arr;
	uint[][] arr2;
	function f() public view {
		assert(arr2[0].length == arr[0].length);
		assert(arr2.length == arr.length);
	}
}
