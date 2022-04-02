contract C {
	uint[] arr;
	function f() public view {
		uint x = arr.length;
		uint y = x;
		assert(arr.length == y);
		assert(arr.length != y);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (120-143='assert(arr.length != y)'): CHC: Assertion violation happens here.\nCounterexample:\narr = []\nx = 0\ny = 0\n\nTransaction trace:\nC.constructor()\nState: arr = []\nC.f()
