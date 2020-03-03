pragma experimental SMTChecker;

contract C {
	function f() public pure {
		(2);
	}
}
// ----
// Warning: (76-79): Statement has no effect.
