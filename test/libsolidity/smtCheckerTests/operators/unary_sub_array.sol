pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function p() public { array.push(); }
	function f(uint x) public {
		require(x < array.length);
		array[x] = 5;
		uint a = --array[x];
		assert(array[x] == 4);
		assert(a == 4);
		uint b = array[x]--;
		assert(array[x] == 3);
		// Should fail.
		assert(b > 4);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (308-321): CHC: Assertion violation happens here.
