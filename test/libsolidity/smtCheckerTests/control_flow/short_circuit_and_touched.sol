contract C
{
	bool b;
	function f() public {
		if ((b = false) && (b == true)) {}
		if ((b == false) && (b = true)) {}
		if ((b = false) && (b = true)) {}
		if ((b == false) && (b == true)) {}
		if ((b = true) && b) {}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (51-77='(b = false) && (b == true)'): BMC: Condition is always false.
// Warning 6838: (88-114='(b == false) && (b = true)'): BMC: Condition is always true.
// Warning 6838: (125-150='(b = false) && (b = true)'): BMC: Condition is always false.
// Warning 6838: (161-188='(b == false) && (b == true)'): BMC: Condition is always false.
// Warning 6838: (199-214='(b = true) && b'): BMC: Condition is always true.
