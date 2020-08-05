pragma experimental SMTChecker;

contract C {
	uint[] arr;
	function f() public view {
		assert(arr.length == g().length);
	}
	function g() internal pure returns (uint[] memory) {
	}
}
