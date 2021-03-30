pragma experimental SMTChecker;

contract C
{
	uint[] array;
	function p() public {
		array.push();
	}
	function f(uint x) public {
		require(x < array.length);
		array[x] = 2;
		uint a = ++array[x];
		assert(array[x] == 3);
		assert(a == 3);
		uint b = array[x]++;
		assert(array[x] == 4);
		// Should fail.
		assert(b < 3);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (311-324): CHC: Assertion violation happens here.
