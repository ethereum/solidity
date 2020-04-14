pragma experimental SMTChecker;

contract C {
	uint[] arr;
	uint[] arr2;
	function f() public {
		arr2 = arr;
		assert(arr2.length == arr.length);
	}
}
