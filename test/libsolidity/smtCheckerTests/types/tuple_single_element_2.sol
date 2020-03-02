pragma experimental SMTChecker;

contract C {
	function f() public pure {
		(("", ""));
	}
}
// ----
// Warning: (76-86): Statement has no effect.
