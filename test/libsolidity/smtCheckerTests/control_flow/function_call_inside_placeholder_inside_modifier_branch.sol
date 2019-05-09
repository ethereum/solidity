pragma experimental SMTChecker;

contract C
{
	modifier m {
		if (true)
			_;
	}

	function f(address a) m public pure {
		if (true) {
			a = g();
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
// Warning: (247-257): Type conversion is not yet fully supported and might yield false positives.
// Warning: (162-172): Type conversion is not yet fully supported and might yield false positives.
// Warning: (247-257): Type conversion is not yet fully supported and might yield false positives.
