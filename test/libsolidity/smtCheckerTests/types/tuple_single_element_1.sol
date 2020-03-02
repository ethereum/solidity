pragma experimental SMTChecker;

contract C {
	function f() public pure {
		(("", 2));
	}
}
// ----
// Warning: (76-85): Statement has no effect.
