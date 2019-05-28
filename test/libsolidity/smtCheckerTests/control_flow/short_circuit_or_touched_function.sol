pragma experimental SMTChecker;

contract C
{
	bool b;
	function g(bool _b) internal returns (bool) {
		b = _b;
		return b;
	}
	function f() public {
		if (g(true) || (b == false)) {}
		if ((b == true) || g(false)) {}
		if (g(true) || g(false)) {}
		if (g(true) || (b == false)) {}
		if (g(false) || b) {}
	}
}
// ----
// Warning: (156-179): Condition is always true.
// Warning: (190-213): Condition is always true.
// Warning: (224-243): Condition is always true.
// Warning: (254-277): Condition is always true.
// Warning: (288-301): Condition is always false.
