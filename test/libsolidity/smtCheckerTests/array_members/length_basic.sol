pragma experimental SMTChecker;

contract C {
	uint[] arr;
	function f() public view {
		uint x = arr.length;
		uint y = x;
		assert(arr.length == y);
		assert(arr.length != y);
	}
}
// ----
// Warning 4661: (153-176): Assertion violation happens here
