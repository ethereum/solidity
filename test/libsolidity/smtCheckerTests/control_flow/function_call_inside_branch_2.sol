pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		if (true) {
			address a = g();
			assert(a == address(0));
		}
		else
		{
			address b = g();
			assert(b == address(0));
		}
	}
	function g() public pure returns (address) {
		address a;
		a = address(0);
		return a;
	}
}
// ----
// Warning: (271-281): Type conversion is not yet fully supported and might yield false positives.
// Warning: (123-133): Type conversion is not yet fully supported and might yield false positives.
// Warning: (271-281): Type conversion is not yet fully supported and might yield false positives.
// Warning: (186-196): Type conversion is not yet fully supported and might yield false positives.
// Warning: (271-281): Type conversion is not yet fully supported and might yield false positives.
