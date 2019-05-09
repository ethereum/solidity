pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		if (true) {
			address a = g();
			assert(a == address(0));
		}
	}
	function g() public pure returns (address) {
		address a;
		a = address(0);
		return a;
	}
}
// ----
// Warning: (208-218): Type conversion is not yet fully supported and might yield false positives.
// Warning: (123-133): Type conversion is not yet fully supported and might yield false positives.
// Warning: (208-218): Type conversion is not yet fully supported and might yield false positives.
