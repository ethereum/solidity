contract C
{
	uint[] array;
	constructor() {
		q(); q();
	}
	function q() public {
		array.push();
	}
	function f(uint x, uint p) public {
		require(p < array.length);
		require(x < 100);
		array[p] = 100;
		array[p] += array[p] + x;
		assert(array[p] < 300);
		assert(array[p] < 110);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (262-284): CHC: Assertion violation happens here.\nCounterexample:\narray = [299, 0]\nx = 99\np = 0\n\nTransaction trace:\nC.constructor()\nState: array = [0, 0]\nC.f(99, 0)
