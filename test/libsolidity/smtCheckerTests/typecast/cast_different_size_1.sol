contract C
{
	function f() public pure {
		bytes2 a = 0x1234;
		uint32 b = uint16(a); // b will be 0x00001234
		assert(b == 0x1234);
		uint32 c = uint32(bytes4(a)); // c will be 0x12340000
		assert(c == 0x12340000);
		uint8 d = uint8(uint16(a)); // d will be 0x34
		assert(d == 0x34);
		uint8 e = uint8(bytes1(a)); // e will be 0x12
		assert(e == 0x12);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
