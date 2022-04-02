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
// ====
// SMTEngine: all
// ----
// Warning 6838: (123-146='g(true) || (b == false)'): BMC: Condition is always true.
// Warning 6838: (157-180='(b == true) || g(false)'): BMC: Condition is always true.
// Warning 6838: (191-210='g(true) || g(false)'): BMC: Condition is always true.
// Warning 6838: (221-244='g(true) || (b == false)'): BMC: Condition is always true.
// Warning 6838: (255-268='g(false) || b'): BMC: Condition is always false.
