pragma experimental SMTChecker;

contract C {
	function f() public pure {
		(2);
	}
}
// ----
// Warning 6133: (76-79): Statement has no effect.
