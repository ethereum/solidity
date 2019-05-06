pragma experimental SMTChecker;

contract C
{
	modifier m(address a) {
		if (true) {
			a = g();
			_;
			assert(a == address(0));
		}
	}

	function f(address a) m(a) public pure {
	}
	function g() public pure returns (address) {
		address a;
		a = address(0);
		return a;
	}
}
// ----
// Warning: (249-259): Type conversion is not yet fully supported and might yield false positives.
// Warning: (118-128): Type conversion is not yet fully supported and might yield false positives.
// Warning: (249-259): Type conversion is not yet fully supported and might yield false positives.
