pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function p() public { array.push(); }
	function f(uint x, uint y) public {
		require(x < array.length);
		array[x] = 200;
		require(x == y);
		assert(array[y] > 300);
	}
}
// ----
// Warning 6328: (205-227): CHC: Assertion violation happens here.\nCounterexample:\narray = [200]\nx = 0\ny = 0\n\nTransaction trace:\nC.constructor()\nState: array = []\nC.p()\nState: array = [0]\nC.f(0, 0)
