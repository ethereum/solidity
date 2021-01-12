pragma experimental SMTChecker;

contract C
{
	function f(uint[] memory array, uint x, uint y) public pure {
		array[x] = 200;
		require(x == y);
		assert(array[y] > 300);
	}
}
// ----
// Warning 6328: (148-170): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 38\ny = 38\n\nTransaction trace:\nC.constructor()\nC.f(array, 38, 38)
