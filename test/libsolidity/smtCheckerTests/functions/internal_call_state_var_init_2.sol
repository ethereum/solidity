contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
	}
	bool b = (f() > 0) || (f() > 0);
}
// ====
// SMTEngine: all
// ----
// Warning 6321: (54-58): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
