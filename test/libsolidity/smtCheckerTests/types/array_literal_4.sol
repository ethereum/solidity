contract C
{
	function f(bool c) public pure {
		uint[3] memory a = c ? [uint(1), 2, 3] : [uint(1), 2, 4];
		uint[3] memory b = [uint(1), 2, c ? 3 : 4];
		assert(a[0] == b[0]);
		assert(a[1] == b[1]);
		assert(a[2] == b[2]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 9 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
