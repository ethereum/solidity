pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		bytes2 a = 0x1234;
		bytes1 b = bytes1(a); // b will be 0x12
		// False positive since truncation is not supported yet.
		assert(b == 0x12);
	}
}
// ----
// Warning: (108-117): Type conversion is not yet fully supported and might yield false positives.
// Warning: (198-215): Assertion violation happens here
