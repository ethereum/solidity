pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		bytes2 a = 0x1234;
		bytes4 b = bytes4(a); // b will be 0x12340000
		// False positive since right padding is not supported yet.
		assert(b == 0x12340000);
		// This should fail (right padding).
		assert(a == b);
	}
}
// ----
// Warning 6328: (207-230): Assertion violation happens here
// Warning 6328: (273-287): Assertion violation happens here
// Warning 5084: (108-117): Type conversion is not yet fully supported and might yield false positives.
