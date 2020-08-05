pragma experimental SMTChecker;

contract C {
	function f() public pure {
		(("", 2));
	}
}
// ----
// Warning 6133: (76-85): Statement has no effect.
