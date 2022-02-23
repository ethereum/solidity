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
