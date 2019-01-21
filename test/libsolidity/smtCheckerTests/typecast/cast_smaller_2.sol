pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint32 a = 0x12345678;
		uint16 b = uint16(a); // b will be 0x5678 now
		// False positive since truncation is not supported yet.
		assert(b == 0x5678);
	}
}
// ----
// Warning: (112-121): Type conversion is not yet fully supported and might yield false positives.
// Warning: (208-227): Assertion violation happens here
