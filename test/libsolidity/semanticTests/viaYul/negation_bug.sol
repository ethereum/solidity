contract C {
	function f() public pure {
		-(int8(0));
		unchecked {
			// Used to incorrectly use the checked unary negation function and revert.
			(-(type(int8).min));
		}
	}
}
// ====
// compileViaYul: also
// ----
// f() ->
