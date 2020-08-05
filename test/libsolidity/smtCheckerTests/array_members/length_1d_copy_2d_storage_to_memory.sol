pragma experimental SMTChecker;
pragma experimental ABIEncoderV2;

contract C {
	uint[][] arr;
	function f() public view {
		uint[][] memory arr2 = arr;
		assert(arr2[0].length == arr[0].length);
		assert(arr2.length == arr.length);
	}
}
