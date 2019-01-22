pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint16 a = 0x1234;
		uint32 b = uint32(a); // b will be 0x00001234 now
		assert(a != b);
	}
}
// ----
// Warning: (108-117): Type conversion is not yet fully supported and might yield false positives.
// Warning: (149-163): Assertion violation happens here
