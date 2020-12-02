pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x, uint p) public {
		require(x < 10);
		array[p] = 10;
		array[p] *= array[p] + x;
		assert(array[p] <= 190);
		assert(array[p] < 50);
	}
}
// ----
// Warning 6328: (191-212): CHC: Assertion violation happens here.\nCounterexample:\narray = []\nx = 0\np = 38\n\n\nTransaction trace:\nconstructor()\nState: array = []\nf(0, 38)
