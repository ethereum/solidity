pragma experimental SMTChecker;

contract C
{
	function f(uint[] memory array, uint x, uint y) public pure {
		require(x < array.length);
		array[x] = 200;
		require(x == y);
		assert(array[y] > 300);
	}
}
// ----
// Warning 6328: (177-199): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 7719\ny = 7719\n\nTransaction trace:\nC.constructor()\nC.f(array, 7719, 7719)
