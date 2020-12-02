pragma experimental SMTChecker;

contract C
{
	uint[][] array;
	function f(uint x, uint y, uint z, uint t) public view {
		// TODO change to = 200 when 2d assignments are supported.
		require(array[x][y] < 200);
		require(x == z && y == t);
		assert(array[z][t] > 300);
	}
}
// ----
// Warning 6328: (243-268): CHC: Assertion violation happens here.\nCounterexample:\narray = []\nx = 38\ny = 7719\nz = 38\nt = 7719\n\n\nTransaction trace:\nconstructor()\nState: array = []\nf(38, 7719, 38, 7719)
