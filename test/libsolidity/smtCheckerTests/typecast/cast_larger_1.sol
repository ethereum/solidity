contract C
{
	function f(uint8 x) public pure {
		uint16 y = uint16(x);
		// True because of x's type
		assert(y < 300);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
