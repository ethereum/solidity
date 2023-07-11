contract C
{
	function f() public pure {
		uint[3] memory array = [uint(1), 2, 3];
		assert(array[0] == 1);
		assert(array[1] == 2);
		assert(array[2] == 3);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
