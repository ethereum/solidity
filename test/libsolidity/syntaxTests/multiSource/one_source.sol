==== Source: SourceName ====
contract A {
	uint256 x;
	function f() public pure { x = 42; }
}
// ----
// TypeError 8961: (SourceName:53-54): Function declared as pure, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
