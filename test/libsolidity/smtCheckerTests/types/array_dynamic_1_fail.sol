pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x, uint y) public {
		array[x] = 200;
		require(x == y);
		assert(array[y] > 300);
	}
}
// ----
// Warning 6328: (137-159): CHC: Assertion violation happens here.\nCounterexample:\narray = []\nx = 38\ny = 38\n\n\nTransaction trace:\nconstructor()\nState: array = []\nf(38, 38)
