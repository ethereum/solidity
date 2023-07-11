contract C
{
	function f() public pure {
		uint32 a = 0x12345678;
		uint16 b = uint16(a); // b will be 0x5678 now
		assert(b == 0x5678);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
