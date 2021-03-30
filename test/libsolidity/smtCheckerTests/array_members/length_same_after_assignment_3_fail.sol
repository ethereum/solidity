pragma experimental SMTChecker;

contract C {
	uint[][] arr;
	constructor() {
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
	}
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
// Warning 6328: (352-378): CHC: Assertion violation happens here.\nCounterexample:\narr = [[], [], [], [], [], [], [], [], []]\nx = 0\ny = 0\nz = 9\nt = 0\n\nTransaction trace:\nC.constructor()\nState: arr = [[], [], [], [], [], [], [], [], []]\nC.f()
// Warning 6328: (382-408): CHC: Assertion violation happens here.\nCounterexample:\narr = [[], [], [], [], [], [], [], [], []]\nx = 0\ny = 0\nz = 9\nt = 0\n\nTransaction trace:\nC.constructor()\nState: arr = [[], [], [], [], [], [], [], [], []]\nC.f()
// Warning 6328: (412-435): CHC: Assertion violation happens here.\nCounterexample:\narr = [[], [], [], [], [], [], [], [], []]\nx = 0\ny = 0\nz = 9\nt = 0\n\nTransaction trace:\nC.constructor()\nState: arr = [[], [], [], [], [], [], [], [], []]\nC.f()
// Warning 6328: (439-465): CHC: Assertion violation happens here.\nCounterexample:\narr = [[], [], [], [], [], [], [], [], []]\nx = 0\ny = 0\nz = 9\nt = 0\n\nTransaction trace:\nC.constructor()\nState: arr = [[], [], [], [], [], [], [], [], []]\nC.f()
