pragma experimental SMTChecker;
contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
	}
	bool b = (f() > 0) || (f() > 0);
}
// ----
// Warning 6321: (86-90): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
