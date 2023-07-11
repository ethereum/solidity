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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (278-291): CHC: Assertion violation happens here.
// Info 1391: CHC: 10 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
