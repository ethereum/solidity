contract C
{
	function f(uint[] memory array, uint x, uint y) public pure {
		require(x < array.length);
		array[x] = 200;
		require(x == y);
		assert(array[y] > 300);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (144-166): CHC: Assertion violation happens here.\nCounterexample:\n\narray = [9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 12, 9, 9, 9, 9, 9, 9, 9, 9, 21, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 200]\nx = 38\ny = 38\n\nTransaction trace:\nC.constructor()\nC.f([9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 12, 9, 9, 9, 9, 9, 9, 9, 9, 21, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 199], 38, 38)
