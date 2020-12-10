pragma experimental SMTChecker;
contract C {
	uint[] array;
	function f(uint x, uint p) public {
		require(x == 2);
		array[p] = 10;
		array[p] /= array[p] / x;
		assert(array[p] == x);
		assert(array[p] == 0);
	}
}
// ----
// Warning 6328: (188-209): CHC: Assertion violation happens here.\nCounterexample:\narray = []\nx = 2\np = 0\n\n\nTransaction trace:\nconstructor()\nState: array = []\nf(2, 0)
