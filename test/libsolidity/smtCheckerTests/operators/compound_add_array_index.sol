pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function f(uint x, uint p) public {
		require(x < 100);
		array[p] = 100;
		array[p] += array[p] + x;
		assert(array[p] < 300);
		assert(array[p] < 110);
	}
}
// ----
// Warning 6328: (192-214): Assertion violation happens here
