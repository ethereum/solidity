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
// Warning 6328: (222-248): CHC: Assertion violation happens here.\nCounterexample:\narr = [], arr2 = []\n\n\n\nTransaction trace:\nconstructor()\nState: arr = [], arr2 = []\nf()
// Warning 6328: (252-278): CHC: Assertion violation happens here.\nCounterexample:\narr = [], arr2 = []\n\n\n\nTransaction trace:\nconstructor()\nState: arr = [], arr2 = []\nf()
// Warning 6328: (282-305): CHC: Assertion violation happens here.\nCounterexample:\narr = [], arr2 = []\n\n\n\nTransaction trace:\nconstructor()\nState: arr = [], arr2 = []\nf()
// Warning 6328: (309-335): CHC: Assertion violation happens here.\nCounterexample:\narr = [], arr2 = []\n\n\n\nTransaction trace:\nconstructor()\nState: arr = [], arr2 = []\nf()
