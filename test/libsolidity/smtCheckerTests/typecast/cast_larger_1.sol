pragma experimental SMTChecker;

contract C
{
	function f(uint8 x) public pure {
		uint16 y = uint16(x);
		// True because of x's type
		assert(y < 300);
	}
}
// ----
// Warning: (94-103): Type conversion is not yet fully supported and might yield false positives.
