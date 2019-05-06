pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		if (true) {
		} else {
			address a = g();
			assert(a == address(0));
		}
	}
	function g() public pure returns (address) {
		address x;
		x = address(0);
		return x;
	}
}
// ----
// Warning: (219-229): Type conversion is not yet fully supported and might yield false positives.
// Warning: (134-144): Type conversion is not yet fully supported and might yield false positives.
// Warning: (219-229): Type conversion is not yet fully supported and might yield false positives.
