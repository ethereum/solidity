contract C {
	function f2() public pure returns(int) {
		int a;
		(((, a),)) = ((1, 2), 3);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6321: (48-51): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
