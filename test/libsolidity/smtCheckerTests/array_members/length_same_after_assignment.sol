pragma experimental SMTChecker;

contract C {
	uint[] arr;
	function f() public view {
		uint[] memory arr2 = arr;
		arr2[2] = 3;
		assert(arr.length == arr2.length);
	}
}
