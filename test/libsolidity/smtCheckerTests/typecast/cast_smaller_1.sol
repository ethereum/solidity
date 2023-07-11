contract C
{
	function f(uint16 x) public pure {
		uint8 y = uint8(x);
		// True because of y's type
		assert(y < 300);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
