pragma experimental SMTChecker;

contract C {
	uint[][] arr;
	uint[][] arr2;
	function f() public {
		uint x = arr[2].length;
		uint y = arr[3].length;
		uint z = arr.length;
		uint t = arr[5].length;
		arr[5] = arr[8];
		assert(arr[2].length != x);
		assert(arr[3].length != y);
		assert(arr.length != z);
		assert(arr[5].length != t);
	}
}
// ----
// Warning: (222-248): Assertion violation happens here
// Warning: (252-278): Assertion violation happens here
// Warning: (282-305): Assertion violation happens here
// Warning: (309-335): Assertion violation happens here
