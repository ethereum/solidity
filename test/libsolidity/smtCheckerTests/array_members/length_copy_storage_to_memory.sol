pragma experimental SMTChecker;

contract C {
	uint[] arr;
	function f() public view {
		uint[] memory marr = arr;
		assert(marr.length == arr.length);
	}
}
