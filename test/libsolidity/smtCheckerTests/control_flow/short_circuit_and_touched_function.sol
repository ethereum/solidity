pragma experimental SMTChecker;

contract C
{
	bool b;
	function g(bool _b) internal returns (bool) {
		b = _b;
		return b;
	}
	function f() public {
		if (g(false) && (b == true)) {}
		if ((b == false) && g(true)) {}
		if (g(false) && g(true)) {}
		if (g(false) && (b == true)) {}
		if (g(true) && b) {}
	}
}
// ----
// Warning: (156-179): Condition is always false.
// Warning: (190-213): Condition is always true.
// Warning: (224-243): Condition is always false.
// Warning: (254-277): Condition is always false.
// Warning: (288-300): Condition is always true.
