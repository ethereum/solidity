pragma experimental SMTChecker;
contract C {
	function f2() public pure returns(int) {
		int a;
		((((((, a)))),)) = ((1, 2), 3);
	}
}
// ----
// Warning 6321: (80-83): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
