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
// ====
// SMTEngine: all
// ----
// Warning 6838: (123-146): BMC: Condition is always false.
// Warning 6838: (157-180): BMC: Condition is always true.
// Warning 6838: (191-210): BMC: Condition is always false.
// Warning 6838: (221-244): BMC: Condition is always false.
// Warning 6838: (255-267): BMC: Condition is always true.
