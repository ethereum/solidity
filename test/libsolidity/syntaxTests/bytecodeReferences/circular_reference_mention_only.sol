contract C {
	function foo() public pure { D; }
}
contract D {
	function foo() public pure { C; }
}
// ----
// Warning 6133: (43-44): Statement has no effect.
// Warning 6133: (93-94): Statement has no effect.
