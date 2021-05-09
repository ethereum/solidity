contract C
{
	uint[][] array;
	function a() public {
		array.push().push();
	}
	function f(uint x, uint y, uint z, uint t) public {
		require(x < array.length);
		require(y < array[x].length);
		array[x][y] = 200;
		require(x == z && y == t);
		assert(array[z][t] > 300);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (245-270): CHC: Assertion violation happens here.\nCounterexample:\narray = [[200]]\nx = 0\ny = 0\nz = 0\nt = 0\n\nTransaction trace:\nC.constructor()\nState: array = []\nC.a()\nState: array = [[0]]\nC.f(0, 0, 0, 0)
