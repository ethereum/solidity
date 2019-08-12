pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		bytes2 a = 0x1234;
		uint32 b = uint16(a); // b will be 0x00001234
		assert(b == 0x1234);
		uint32 c = uint32(bytes4(a)); // c will be 0x12340000
		// This fails because right padding is not supported.
		assert(c == 0x12340000);
		uint8 d = uint8(uint16(a)); // d will be 0x34
		// False positive since truncating is not supported yet.
		assert(d == 0x34);
		uint8 e = uint8(bytes1(a)); // e will be 0x12
		// False positive since truncating is not supported yet.
		assert(e == 0x12);
	}
}
// ----
// Warning: (186-195): Type conversion is not yet fully supported and might yield false positives.
// Warning: (317-333): Type conversion is not yet fully supported and might yield false positives.
// Warning: (451-460): Type conversion is not yet fully supported and might yield false positives.
// Warning: (280-303): Assertion violation happens here
// Warning: (414-431): Assertion violation happens here
// Warning: (542-559): Assertion violation happens here
