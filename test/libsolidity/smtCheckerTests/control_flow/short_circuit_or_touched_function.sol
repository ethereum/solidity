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
// Warning 6838: (156-179): BMC: Condition is always true.
// Warning 6838: (190-213): BMC: Condition is always true.
// Warning 6838: (224-243): BMC: Condition is always true.
// Warning 6838: (254-277): BMC: Condition is always true.
// Warning 6838: (288-301): BMC: Condition is always false.
