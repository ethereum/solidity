pragma experimental SMTChecker;

contract C
{
	address owner;
	modifier m {
		if (true)
			owner = g();
		_;
	}
	function f() m public {
	}
	function g() public pure returns (address) {
		address a;
		a = address(0);
		return a;
	}
}
// ----
// Warning: (205-215): Type conversion is not yet fully supported and might yield false positives.
// Warning: (205-215): Type conversion is not yet fully supported and might yield false positives.
