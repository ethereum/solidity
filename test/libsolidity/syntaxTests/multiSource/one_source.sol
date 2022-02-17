==== Source: SourceName ====
contract A {
	uint256 x;
	function f() public pure { x = 42; }
}
// ----
// TypeError 8961: (SourceName:53-54): Function cannot be declared as pure because this expression (potentially) modifies the state.
