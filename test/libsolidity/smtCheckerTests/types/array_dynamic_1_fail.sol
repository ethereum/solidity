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
// ====
// SMTEngine: all
// ----
// Warning 6328: (172-194): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
