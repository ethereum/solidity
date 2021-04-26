contract C
{
	uint[10] array;
	function f(uint x, uint y) public {
		require(x < array.length);
		array[x] = 200;
		require(x == y);
		assert(array[y] > 300);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (135-157): CHC: Assertion violation happens here.\nCounterexample:\narray = [0, 0, 0, 0, 0, 0, 0, 0, 0, 200]\nx = 9\ny = 9\n\nTransaction trace:\nC.constructor()\nState: array = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]\nC.f(9, 9)
