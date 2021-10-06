abstract contract I {
	function f() external virtual;
}
contract C is I {
	function f() external {}
}
// ----
// TypeError 9456: (75-99): Overriding function is missing "override" specifier.
